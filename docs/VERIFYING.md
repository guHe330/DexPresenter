# Verifying a download

Every release page lists a SHA-256 for each artifact. Compare it with the hash of the file you downloaded:

```powershell
Get-FileHash DexPresenter_1.0.0_Setup.exe -Algorithm SHA256
```

Both artifacts also carry a [GitHub build provenance attestation](https://docs.github.com/actions/security-guides/using-artifact-attestations-to-establish-provenance-for-builds), which ties them to the workflow run and commit that built them. With the [GitHub CLI](https://cli.github.com/):

```powershell
gh attestation verify DexPresenter_1.0.0_Setup.exe --repo guHe330/DexPresenter
```

The attestation checks the supply chain. It is not a Windows code signature, so Windows SmartScreen still warns on first run.
