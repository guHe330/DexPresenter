# DexPresenter ${VERSION}

> **Alpha** — This project is in early development. Expect breaking changes, missing features, and rough edges.

## Downloads

| File | Description |
| ---- | ----------- |
| `DexPresenter_${VERSION}_Setup.exe` | Installer (recommended). Includes the source archive per GPLv3. |
| `Portable_DexPresenter.zip` | Portable version — unzip and run, no installation. |

> **Note on SmartScreen:** DexPresenter is distributed without a commercial code-signing certificate, so Windows SmartScreen may warn on first run. Click **More info** → **Run anyway**.

### Verifying these downloads

Every file above is published with a [GitHub build attestation](https://docs.github.com/actions/security-guides/using-artifact-attestations-to-establish-provenance-for-builds), so you can confirm it was built by this repository's release workflow from the `v${VERSION}` tag rather than assembled somewhere else. With the [GitHub CLI](https://cli.github.com/) installed:

```
gh attestation verify DexPresenter_${VERSION}_Setup.exe --repo guHe330/DexPresenter
```

The same works for `Portable_DexPresenter.zip` and for the `DexPresenter.exe` unpacked from it. This is a supply-chain check, not a Windows signature — it does not affect the SmartScreen prompt above.

## What's New

<!-- Fill in per release, or replace this template usage with generated notes -->
- See the commit history for changes in this release.

## Privacy

No telemetry, no network calls, no data collection. See [PRIVACY.md](https://github.com/guHe330/DexPresenter/blob/main/PRIVACY.md).
