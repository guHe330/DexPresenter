# Contributing to DexPresenter

Thanks for your interest in DexPresenter. Bug reports, test reports, feature suggestions, and pull requests are all welcome.

> **A personal project, shared openly.** DexPresenter is built and maintained by a single developer as a passion project. It's not backed by a company or a team, just one person who kept forgetting ZoomIt's hotkeys. Development follows my own priorities and pace.

## What you can expect from me

I work on DexPresenter in my spare time, around a full-time life. That shapes a few ground rules:

- **Reviews happen when they happen.** Days, sometimes longer. Small, focused changes move fastest.
- **I may decline a pull request.** A change might not fit the direction, the scope, or my plans for the project. That isn't a judgment of your work — it's me keeping a personal project the way I want it. Open an issue first for anything larger than a fix, and you'll save yourself an evening.
- **Not every issue gets a reply.** Reports are genuinely appreciated and I read what I can, but capacity is finite.
- **DexPresenter is GPLv3 — so fork freely.** If you need a change I haven't picked up, the [GPLv3 license](../LICENSE) gives you the right to take the project in your own direction. That's the point of free software. Keep your fork GPLv3-compliant and clearly distinct from the official DexPresenter builds.

## Reporting bugs

Open an [issue](https://github.com/guHe330/DexPresenter/issues/new/choose) and pick a template. Check the [open bugs](https://github.com/guHe330/DexPresenter/issues?q=is%3Aissue+label%3Abug) first to avoid duplicates. The forms ask for what is actually needed to reproduce a problem:

- Your Windows version (e.g. Windows 11 Pro 24H2)
- Your DexPresenter version (shown in the installer name, the release page, or the exe's file properties)
- What you did, what you expected, and what happened instead
- A screenshot, if the problem is visual
- Your display setup, if more than one monitor or display scaling is involved

A partial report beats no report. If a field does not apply, leave it empty and post anyway.

For toolbar position, settings, or persistence bugs, attaching `%AppData%\DexPresenter\config.ini` is the fastest route to a fix.

**Windows 11 only.** DexPresenter is developed and tested on Windows 11. Reports from Windows 10 are out of scope.

## Testing is a contribution

You do not need to write code to help, and right now testing is worth more than code.

DexPresenter is alpha software, and development covers exactly one desk: a handful of monitors at one set of scaling factors. Everything beyond that is untested territory, and the parts most likely to break are the ones that depend on your hardware:

- **Multiple monitors.** The toolbar remembers a position per display device and has to survive monitors being unplugged, rearranged, or turned off. Mixed arrangements, hot-plug order, and monitors that vanish while the toolbar is on them all need far wider coverage than they get here. The [Multi-monitor / DPI test report](https://github.com/guHe330/DexPresenter/issues/new?template=multi_monitor_report.yml) form has a checklist; one completed section is a useful report.
- **Mixed DPI**, such as a 150% laptop panel next to a 100% external screen. The Draw, Zoom, and Snip overlays capture the whole screen, so scaling bugs show up as offset strokes or a cropped capture.
- **Unusual display hardware**: ultrawides, portrait orientation, TVs, wireless displays, docks, and anything that changes resolution while DexPresenter is running.
- **Real presentations.** Projectors and meeting-room screens duplicate or extend the desktop mid-talk, which is precisely when the tool has to behave.

Reports that everything worked are worth posting too. Knowing the toolbar survives a three-monitor unplug is as useful as knowing it does not.

## Suggesting a feature

Check the [existing requests](https://github.com/guHe330/DexPresenter/issues?q=is%3Aissue+label%3Aenhancement) first, then open an issue describing the **problem** you're trying to solve, not just the solution you have in mind. I can't promise it will be built, but well-reasoned suggestions are useful input.

## Building

Requires Visual Studio 2022 with the C++ workload and CMake 3.21+. See [Building from Source](../README.md#building-from-source) for the full set of options.

```powershell
.\build.ps1
```

## Pull requests

- Branch from `main` and keep each pull request to one logical change.
- Match the existing C++23 / Win32 style in `src/`; a `.clang-format` is provided.
- Build cleanly at `/W4` with MSVC, and ideally with clang-cl too (`.\build.ps1 -Clang`). CI checks both.
- The unit tests must pass (`.\build.ps1` runs them).
- Every source file carries the GPL header from [COPYRIGHT.txt](../COPYRIGHT.txt). New files need it too — CI enforces this.
- Explain *why* the change is worth making, not just what it does. If it changes something visible, a before/after screenshot helps.
- User-visible changes get a line in the changelog (see below).

## Changelog

**Your part:** if the change is visible to a user, add one line under the `## [Unreleased]` heading at the top of [CHANGELOG.md](CHANGELOG.md), in the matching `### Added` / `### Changed` / `### Fixed` group (create the group if it is missing).

Keep it to one or two sentences: what changed, and why if that is not obvious. No symbol names, no file paths, no implementation detail — that belongs in the commit message. Do not add a version heading, do not edit already-released sections, and do not touch anything under `Distribute/`.

**Maintainer's part:** versioning, the dated release section, tagging, and publishing the GitHub Release. The release workflow builds the release notes straight from the changelog section for that version, so a release cannot ship without one.

If you are not sure a change is user-visible, add the line anyway; it is easier to drop one than to reconstruct it at release time.

## Contributor License Agreement

Before a pull request can be merged, you need to sign the [Contributor License Agreement](CLA.md): three lines, and you keep your copyright.

It is inbound-equals-outbound. Your contribution is licensed under the **GPLv3**, the same licence as the project, you confirm you have the right to submit it, and the copyright stays yours. DexPresenter does not ask for a relicensing grant, and it will not be relicensed out from under the people who helped build it.

This is a one-time, automated step. When you open your first pull request, a bot comments with a link. To sign, reply on the pull request with exactly:

```
I have read the CLA Document and I hereby sign the CLA
```

Your signature is recorded in `signatures/version1/cla.json` and covers all your future contributions, so you will not be asked again.

If you contribute as part of your job, make sure your employer permits it.

## License

DexPresenter is released under the [GNU General Public License v3.0](../LICENSE). Anything you contribute is licensed under the GPLv3 as well.
