# DexPresenter ${VERSION}

> **Alpha.** This project is in early development. Expect breaking changes, missing features, and rough edges.

## What's New

${CHANGELOG}

## Downloads

| File | Description | Size |
| ---- | ----------- | ---- |
| `DexPresenter_${VERSION}_Setup.exe` | Installer (recommended). Includes the source archive. | ${SETUP_SIZE} |
| `Portable_DexPresenter.zip` | Portable version: unzip and run, no installation. Includes the source archive. | ${PORTABLE_SIZE} |
| `DexPresenter_${VERSION}_Sources.zip` | Complete corresponding source (GPLv3). Also inside both packages above. | ${SOURCES_SIZE} |

> Requires Windows 11. DexPresenter is developed and tested on Windows 11 only.

> DexPresenter is distributed without a commercial code-signing certificate, so Windows SmartScreen may warn on first run and report an unknown publisher. Click **More info**, then **Run anyway**. You can verify the download first with either check below.

### Verifying these downloads

SHA-256:

```
DexPresenter_${VERSION}_Setup.exe  ${SETUP_SHA256}
Portable_DexPresenter.zip          ${PORTABLE_SHA256}
```

Check a file in PowerShell:

```powershell
Get-FileHash DexPresenter_${VERSION}_Setup.exe -Algorithm SHA256
```

Both files also carry a [GitHub build attestation](https://docs.github.com/actions/security-guides/using-artifact-attestations-to-establish-provenance-for-builds), so you can confirm they were built by this repository's release workflow from the `v${VERSION}` tag and not assembled somewhere else. With the [GitHub CLI](https://cli.github.com/) installed:

```powershell
gh attestation verify DexPresenter_${VERSION}_Setup.exe --repo guHe330/DexPresenter
```

The same works for `Portable_DexPresenter.zip` and for the `DexPresenter.exe` unpacked from it. This checks the supply chain. It is not a Windows signature and does not affect the SmartScreen prompt above.

## Privacy

No telemetry, no network calls, no data collection. CI fails any build that uses a networking API. See [PRIVACY.md](https://github.com/guHe330/DexPresenter/blob/main/docs/PRIVACY.md).

## Source

DexPresenter is [GPLv3](https://github.com/guHe330/DexPresenter/blob/main/LICENSE). Every package above carries the complete corresponding source it was built from: the installer drops `DexPresenter_Sources.zip` next to the executable, the portable zip contains it, and it is published standalone above. The full history is at [github.com/guHe330/DexPresenter](https://github.com/guHe330/DexPresenter).
