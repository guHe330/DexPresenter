<#
.SYNOPSIS
    Builds the GPL "complete corresponding source" archive.

.DESCRIPTION
    The single owner of what goes into DexPresenter_Sources.zip. Both build.ps1
    and .github/workflows/build-release.yml call this, so the local archive and
    the released one can never disagree -- the file list used to be duplicated in
    both, and had already drifted.

    GPLv3 section 6 requires that anyone receiving a binary can get the source it
    was built from. That makes this archive a licence obligation, not a
    convenience, so the failure modes here are all loud:

      - a listed path that does not exist is an error, not a silent skip
      - build outputs are excluded explicitly, then the finished archive is
        re-opened and checked for binaries that slipped in anyway
      - the result must contain enough to actually rebuild from

.PARAMETER OutputPath
    Where to write the zip. Required.

.PARAMETER SourceRoot
    Repository root. Defaults to the parent of this script's directory.

.EXAMPLE
    ./Distribute/New-SourceArchive.ps1 -OutputPath build/DexPresenter_Sources.zip
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$OutputPath,

    [string]$SourceRoot = (Split-Path -Parent $PSScriptRoot)
)

$ErrorActionPreference = 'Stop'

# Everything a recipient needs to rebuild DexPresenter from scratch, plus the
# licence and notice files that have to travel with it.
$ArchiveItems = @(
    'src'
    'tests'
    'resources'
    'installer'
    'docs'
    'Distribute'
    'CMakeLists.txt'
    'CMakePresets.json'
    'CMakeSettings.json'
    'README.md'
    'LICENSE'
    'COPYRIGHT.txt'
    'THIRD_PARTY_NOTICES.txt'
    'build.ps1'
    '.clang-format'
    '.editorconfig'
    '.gitignore'
)

# Paths above that are optional -- absent ones are skipped rather than failing.
# Everything else must exist, so a rename cannot quietly drop half the source.
$OptionalItems = @(
    'CMakeSettings.json'
    '.clang-format'
    '.editorconfig'
)

# Build products that live inside the directories above. Compress-Archive does
# not read .gitignore, which is how a 7 MB stale installer ended up inside the
# "source" archive: installer/innosetup/output/ is gitignored but was swept in.
$ExcludeDirNames  = @('output', 'build', 'build-clang', '.vs', '_deps', 'Release', 'Debug')
$ExcludeFileGlobs = @('*.exe', '*.dll', '*.zip', '*.obj', '*.pdb', '*.ilk', '*.res', '*.lib', '*.exp')

Push-Location $SourceRoot
try {
    $staging = Join-Path ([System.IO.Path]::GetTempPath()) ("dexpresenter_src_" + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $staging -Force | Out-Null

    try {
        $missing = @()
        foreach ($item in $ArchiveItems) {
            if (-not (Test-Path $item)) {
                if ($OptionalItems -notcontains $item) { $missing += $item }
                continue
            }

            $dest = Join-Path $staging $item
            if (Test-Path $item -PathType Container) {
                # -Exclude on Copy-Item does not recurse usefully, so copy the
                # tree and then prune, which also prunes nested build output.
                Copy-Item $item -Destination $dest -Recurse -Force
            } else {
                Copy-Item $item -Destination $dest -Force
            }
        }

        if ($missing.Count -gt 0) {
            throw ("Source archive is incomplete -- these paths do not exist: " + ($missing -join ', ') +
                   ". Update the list in Distribute/New-SourceArchive.ps1 if something was renamed.")
        }

        foreach ($name in $ExcludeDirNames) {
            Get-ChildItem $staging -Recurse -Directory -Filter $name -ErrorAction SilentlyContinue |
                ForEach-Object { Remove-Item $_.FullName -Recurse -Force -ErrorAction SilentlyContinue }
        }
        foreach ($glob in $ExcludeFileGlobs) {
            Get-ChildItem $staging -Recurse -File -Filter $glob -ErrorAction SilentlyContinue |
                Remove-Item -Force -ErrorAction SilentlyContinue
        }

        $outDir = Split-Path -Parent $OutputPath
        if ($outDir -and -not (Test-Path $outDir)) {
            New-Item -ItemType Directory -Path $outDir -Force | Out-Null
        }
        if (Test-Path $OutputPath) { Remove-Item $OutputPath -Force }

        Compress-Archive -Path (Join-Path $staging '*') -DestinationPath $OutputPath -Force

        # Verify the result rather than trusting the exclusion lists. A binary in
        # here means an exclusion was missed; a missing entry point means the
        # archive would not rebuild, which is the whole obligation.
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        $zip = [System.IO.Compression.ZipFile]::OpenRead((Resolve-Path $OutputPath))
        try {
            $names = $zip.Entries | ForEach-Object { $_.FullName }

            $binaries = $names | Where-Object { $_ -match '\.(exe|dll|zip|obj|pdb|lib)$' }
            if ($binaries) {
                throw ("Build output leaked into the source archive: " + ($binaries -join ', '))
            }

            foreach ($required in @('CMakeLists.txt', 'src/main.cpp', 'LICENSE')) {
                if ($names -notcontains $required) {
                    throw "Source archive is missing $required -- it would not rebuild."
                }
            }

            $sizeMb = [math]::Round((Get-Item $OutputPath).Length / 1MB, 2)
            Write-Host "Source archive: $OutputPath ($sizeMb MB, $($zip.Entries.Count) files)" -ForegroundColor Green
        } finally {
            $zip.Dispose()
        }
    } finally {
        Remove-Item $staging -Recurse -Force -ErrorAction SilentlyContinue
    }
} finally {
    Pop-Location
}
