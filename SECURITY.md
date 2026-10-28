# Security Policy

DexPresenter captures your screen locally on request and never sends data anywhere (see [PRIVACY.md](docs/PRIVACY.md)). It still captures the screen and writes image files, so security problems need a private way to be reported.

## Supported versions

Only the latest release receives security fixes. There are no backports.

## Reporting a vulnerability

Please do not open a public issue for security problems. Report them privately via [GitHub Security Advisories](https://github.com/guHe330/DexPresenter/security/advisories/new) ("Report a vulnerability" on the repository's Security tab).

What to expect:

- This is a one-person spare-time project. Responses are best effort, typically within a couple of weeks.
- There is no bug bounty.
- Confirmed vulnerabilities will be fixed in the next release, and credit given in the release notes unless you prefer otherwise.

## Scope

In scope: the DexPresenter application (`src/`), the installer, and the release pipeline.
Out of scope: issues requiring an already-compromised machine, social engineering, SmartScreen warnings on unsigned builds (known, see the README), and self-built or modified binaries.

## What CI enforces

[`.github/workflows/security-guards.yml`](.github/workflows/security-guards.yml) checks several of this project's public claims on every push and pull request, so a later change cannot quietly break them:

- No networking. The shipped code is scanned for any use of WinHTTP, WinINet, sockets, or comparable APIs, and `CMakeLists.txt` for any networking library. A hit fails the build. This backs the "no server communication of any kind" promise in [PRIVACY.md](docs/PRIVACY.md).
- Pinned actions. Every GitHub Action used anywhere in this repository must be pinned to a full commit SHA. The release job holds `contents: write` and signs build provenance; a mutable tag there would let a compromised upstream action into that job.
- Version integrity. `src/Version.h` is the single source of truth for the version, and the release workflow refuses to publish a tag that disagrees with it. The commit a release is built from therefore always describes the binary it produced.
- GPL headers. Every source file must carry the licence header.

If you find a way around one of these guards, please report it.
