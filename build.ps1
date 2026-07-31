<#
.SYNOPSIS
    Build script for DexPresenter
.DESCRIPTION
    Configures and builds DexPresenter using CMake + MSVC.
    Optionally runs tests and builds the Inno Setup installer.
.PARAMETER Config
    Build configuration: Debug or Release (default: Release)
.PARAMETER SkipTests
    Skip running unit tests after build
.PARAMETER BuildInstaller
    Build the Inno Setup installer after successful build
.PARAMETER Clean
    Remove the build directory before building
.PARAMETER Clang
    Build with clang-cl instead of MSVC (output goes to build-clang\).
    Requires the "C++ Clang tools for Windows" VS component or an LLVM install.
#>
param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",
    [switch]$SkipTests,
    [switch]$BuildInstaller,
    [switch]$Clean,
    [switch]$Clang
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location $scriptDir

# --- Find Visual Studio ---
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    Write-Error "vswhere not found. Install Visual Studio 2022 with C++ workload."
    exit 1
}

$vsPath = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    Write-Error "Visual Studio with C++ workload not found."
    exit 1
}
Write-Host "Using Visual Studio at: $vsPath" -ForegroundColor Cyan

# --- Clean ---
# clang-cl builds go to a separate directory so the two toolchains never
# share a CMake cache (and build\ stays the canonical dir for the installer).
$buildDirName = if ($Clang) { "build-clang" } else { "build" }
$buildDir = Join-Path $scriptDir $buildDirName
if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildDir
}

# --- Kill running DexPresenter ---
$running = Get-Process -Name "DexPresenter" -ErrorAction SilentlyContinue
if ($running) {
    Write-Host "Stopping running DexPresenter..." -ForegroundColor Yellow
    Stop-Process -Name "DexPresenter" -Force
    Start-Sleep -Seconds 1
}

# --- Find CMake ---
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    # Try VS bundled CMake
    $vsCmake = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if (Test-Path $vsCmake) {
        $cmake = $vsCmake
    } else {
        Write-Error "CMake not found. Install CMake or use the one bundled with VS."
        exit 1
    }
} else {
    $cmake = $cmake.Source
}
Write-Host "Using CMake: $cmake" -ForegroundColor Cyan

# --- Pick a generator: Ninja (fast, parallel) if available, else NMake ---
$ninja = Get-Command ninja -ErrorAction SilentlyContinue
if (-not $ninja) {
    # VS bundles Ninja with its CMake tools component
    $vsNinja = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
    if (Test-Path $vsNinja) {
        $env:PATH = "$(Split-Path $vsNinja);$env:PATH"
        $ninja = $vsNinja
    }
}
$generator = if ($ninja) { "Ninja" } else { "NMake Makefiles" }

# --- clang-cl toolchain (optional) ---
$compilerArgs = @()
if ($Clang) {
    $clangCl = Get-Command clang-cl -ErrorAction SilentlyContinue
    if (-not $clangCl) {
        $vsClang = Join-Path $vsPath "VC\Tools\Llvm\x64\bin\clang-cl.exe"
        if (Test-Path $vsClang) {
            $env:PATH = "$(Split-Path $vsClang);$env:PATH"
            $clangCl = $vsClang
        }
    }
    if (-not $clangCl) {
        Write-Error ("clang-cl not found. Install the 'C++ Clang tools for Windows' component " +
            "via the Visual Studio Installer, or LLVM from https://llvm.org and add it to PATH.")
        exit 1
    }
    Write-Host "Using clang-cl: $clangCl" -ForegroundColor Cyan
    $compilerArgs = @("-DCMAKE_C_COMPILER=clang-cl", "-DCMAKE_CXX_COMPILER=clang-cl")
}

# --- Set up MSVC environment ---
$vcvarsall = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"
if (-not (Test-Path $vcvarsall)) {
    Write-Error "vcvarsall.bat not found at: $vcvarsall"
    exit 1
}

# Import MSVC environment
Write-Host "Setting up MSVC x64 environment..." -ForegroundColor Cyan
$envOutput = cmd /c "`"$vcvarsall`" x64 >nul 2>&1 && set"
foreach ($line in $envOutput) {
    if ($line -match "^([^=]+)=(.*)$") {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
    }
}

# --- Configure ---
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Write-Host "`n=== Configuring ($generator, $Config$(if ($Clang) { ', clang-cl' })) ===" -ForegroundColor Green
& $cmake -G $generator "-DCMAKE_BUILD_TYPE=$Config" @compilerArgs -S . -B $buildDirName
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configure failed."
    exit 1
}

# --- Build ---
Write-Host "`n=== Building ===" -ForegroundColor Green
& $cmake --build $buildDirName --config $Config
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed."
    exit 1
}

Write-Host "`nBuild successful!" -ForegroundColor Green

# --- Tests ---
if (-not $SkipTests) {
    Write-Host "`n=== Running Tests ===" -ForegroundColor Green
    & $cmake --build $buildDirName --target DexPresenterTests --config $Config 2>$null
    $testExe = Join-Path $buildDir "DexPresenterTests.exe"
    if (Test-Path $testExe) {
        & $testExe --gtest_color=yes
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Tests failed!"
            exit 1
        }
        Write-Host "All tests passed!" -ForegroundColor Green
    } else {
        Write-Warning "Test executable not found at $testExe"
    }
}

# --- Source Code Archive (for GPL distribution) ---
Write-Host "`n=== Creating Source Code Archive ===" -ForegroundColor Green
$sourcesZip = Join-Path $buildDir "DexPresenter_Sources.zip"

# Remove old archive if it exists
if (Test-Path $sourcesZip) {
    Remove-Item $sourcesZip
}

# Create archive with source files
$archiveItems = @(
    "src",
    "tests",
    "resources",
    "installer",
    "CMakeLists.txt",
    "CMakePresets.json",
    "README.md",
    "LICENSE",
    "PRIVACY.md",
    "build.ps1",
    ".gitignore"
)

# Create a temporary list of files to zip
$tempZipList = Join-Path $env:TEMP "dexpresenter_zip_list.txt"
$archiveItems | ForEach-Object {
    if (Test-Path $_) {
        Add-Content $tempZipList $_
    }
}

# Use PowerShell compression (available in Windows 10+)
if (Get-Command Compress-Archive -ErrorAction SilentlyContinue) {
    # Compress each item individually
    $filesToZip = @()
    foreach ($item in $archiveItems) {
        if (Test-Path $item) {
            $filesToZip += $item
        }
    }

    if ($filesToZip.Count -gt 0) {
        Compress-Archive -Path $filesToZip -DestinationPath $sourcesZip -Force
        if (Test-Path $sourcesZip) {
            $zipSize = (Get-Item $sourcesZip).Length / 1MB
            Write-Host "Source archive created: $sourcesZip ($([math]::Round($zipSize, 1)) MB)" -ForegroundColor Green
        }
    }
} else {
    Write-Warning "Compress-Archive not available. Skipping source archive."
}

if (Test-Path $tempZipList) {
    Remove-Item $tempZipList
}

# --- Installer ---
if ($BuildInstaller -and $Clang) {
    Write-Warning "The installer is built from the MSVC output in build\; skipping installer for a clang build."
    $BuildInstaller = $false
}
if ($BuildInstaller) {
    Write-Host "`n=== Building Installer ===" -ForegroundColor Green
    $iscc = Get-Command iscc -ErrorAction SilentlyContinue
    if (-not $iscc) {
        $isccPath = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
        if (Test-Path $isccPath) { $iscc = $isccPath }
    } else {
        $iscc = $iscc.Source
    }

    if ($iscc) {
        & $iscc /DMyAppVersion="1.0.0" "installer\innosetup\DexPresenter.iss"
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Installer built successfully!" -ForegroundColor Green
        } else {
            Write-Error "Installer build failed."
        }
    } else {
        Write-Warning "Inno Setup not found. Skipping installer."
    }
}

# --- Summary ---
$exePath = Join-Path $buildDir "DexPresenter.exe"
if (Test-Path $exePath) {
    $size = (Get-Item $exePath).Length / 1KB
    Write-Host "`n=== Build Complete ===" -ForegroundColor Green
    Write-Host "  Output: $exePath"
    Write-Host "  Size:   $([math]::Round($size, 1)) KB"
}
