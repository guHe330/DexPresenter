# DexPresenter Privacy Policy

## Summary

DexPresenter does not collect, store, or transmit any personal data or telemetry. Everything the application needs is stored locally on your PC.

## Data storage

DexPresenter stores only the following, all on your computer:

- Installation: `Program Files\DexPresenter\`, the executable and built-in UI resources
- Configuration: `%AppData%\DexPresenter\config.ini`, with the zoom level, timer duration, and tool preferences
- Screenshots: `%USERPROFILE%\Pictures\DexPresenter\` (or your custom directory), the images you capture or annotate
- Registry, for autorun only: if you enable "Start with Windows", one entry at `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run\DexPresenter` pointing to the executable

You can change the screenshot directory in the Settings dialog and disable autorun at any time. You can delete any of these files and registry entries.

## What DexPresenter does not do

- No telemetry or usage tracking
- No personal data collection
- No account or login
- No cloud sync or online services
- No server communication of any kind
- No analytics or crash reporting
- No advertising or third-party trackers
- No update checks (updates are manual)

## Transparency

DexPresenter is open source (GPLv3), so you can read the source code on GitHub and check these claims yourself. The application makes no network calls and depends only on Windows system libraries.

You don't have to audit the "no network calls" part by hand. Every push and pull request runs a [CI guard](../.github/workflows/security-guards.yml) that fails the build if any networking API (WinHTTP, WinINet, sockets, and the like) appears in the shipped code, or if a networking library is linked.

Every release is also published with a GitHub build provenance attestation. It confirms that the binary you downloaded was built by this repository's release workflow from the tagged source and not modified afterwards:

```powershell
gh attestation verify DexPresenter_1.0.0_Setup.exe --repo guHe330/DexPresenter
```

## Questions

If you have privacy concerns or questions, open an issue on the project's GitHub repository.

*Last updated: September 2026*
