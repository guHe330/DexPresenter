# DexPresenter Privacy Policy

## Summary

DexPresenter respects your privacy completely. **We do not collect, store, or transmit any personal data or telemetry.** Everything the application needs lives locally on your PC.

## Data Storage

DexPresenter stores **only** the following locally on your computer:

- **Installation**: `Program Files\DexPresenter\` — The executable and built-in UI resources
- **Configuration**: `%AppData%\DexPresenter\config.ini` — Zoom level, timer duration, and tool preferences
- **Screenshots**: `%USERPROFILE%\Pictures\DexPresenter\` (or your custom directory) — Images you capture or annotate
- **Registry** (Windows autorun only): If you enable "Start with Windows", a single registry entry is created at `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run\DexPresenter` pointing to the executable

You can change the screenshot directory in the Settings dialog, and you can disable autorun at any time. All files and registry entries remain under your full control and can be deleted.

## What We Don't Do

- ❌ No telemetry or usage tracking
- ❌ No personal data collection
- ❌ No account creation or login required
- ❌ No cloud sync or online services
- ❌ No server communication of any kind
- ❌ No analytics or crash reporting
- ❌ No advertising or third-party trackers
- ❌ No updates checking (manual updates only)

## Transparency

Because DexPresenter is open-source (GPLv3), you can inspect the source code on GitHub to verify these claims yourself. The application makes no network calls and has no external dependencies beyond Windows system libraries.

You do not have to take the "no network calls" part on trust, or audit it by hand. Every push and pull request runs a [CI guard](../.github/workflows/security-guards.yml) that fails the build if any networking API (WinHTTP, WinINet, sockets, and the like) appears in the shipped code, or if a networking library is linked. The promise on this page is a build-breaking rule, not just a statement of intent.

Every release is also published with a GitHub build provenance attestation, so you can confirm that the binary you downloaded was built by this repository's release workflow from the tagged source — and not modified in between:

```powershell
gh attestation verify DexPresenter_1.0.0_Setup.exe --repo guHe330/DexPresenter
```

## Questions?

If you have privacy concerns or questions, please open an issue on the project's GitHub repository.

---

*Last updated: September 2026*
