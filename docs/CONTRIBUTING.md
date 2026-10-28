# Contributing to DexPresenter

Thanks for your interest in DexPresenter. Bug reports, feature suggestions, and pull requests are all welcome.

## Reporting bugs and requesting features

Open an [issue](https://github.com/guHe330/DexPresenter/issues/new/choose) and pick a template. The forms ask for what is actually needed to reproduce a problem:

- Your Windows version (e.g. Windows 11 Pro 24H2)
- The DexPresenter version (shown in the installer name, the release page, or the exe's file properties)
- What you did, what you expected, and what happened instead
- A screenshot, if the problem is visual
- Your display setup, if more than one monitor or display scaling is involved

A partial report beats no report. If a field does not apply, leave it empty and post anyway.

**Windows 11 only.** Bugs reported from Windows 10 are closed as out of scope.

For toolbar position, settings, or persistence bugs, attaching `%AppData%\DexPresenter\config.ini` is the fastest route to a fix.

## Testing is a contribution

You do not need to write code to help, and right now testing is worth more than code.

DexPresenter is alpha software, and development covers one desk: a handful of monitors at one set of scaling factors. Anything beyond that is untested. The gaps where reports from other setups help most:

- **Multiple monitors.** The toolbar remembers a position per display device and has to survive monitors being unplugged, rearranged, or turned off. The [Multi-monitor / DPI test report](https://github.com/guHe330/DexPresenter/issues/new?template=multi_monitor_report.yml) form has a checklist; one completed section is a useful report.
- **Mixed DPI**, such as a 150% laptop panel next to a 100% external screen. Scaling bugs show up as offset strokes or a cropped capture in Draw, Zoom, and Snip.
- **Unusual display hardware**: ultrawides, portrait orientation, TVs, wireless displays, docks.
- **Real presentations.** Projectors and meeting-room screens that duplicate or extend the desktop mid-talk.

Reports that everything worked are worth posting too. Knowing the toolbar survives a three-monitor unplug is as useful as knowing it does not.

## Scope, before you write code

DexPresenter is a personal tool. It is developed to fit the maintainer's own presentations, and changes that do not serve that purpose are declined regardless of how well they are implemented. See [Project Scope](../README.md#project-scope) in the README.

So: **open an issue before writing code.** A short "would you take a PR that does X?" costs you five minutes and can save you an evening. Bug fixes are almost always welcome without asking.

Reviews happen when there is time: days, sometimes longer.

## Contributor License Agreement

Before a pull request can be merged, you need to sign the [Contributor License Agreement](CLA.md): three lines, and you keep your copyright.

This is a one-time, automated step. When you open your first pull request, a bot comments with a link. To sign, reply on the pull request with exactly:

```
I have read the CLA Document and I hereby sign the CLA
```

Your signature is recorded in `signatures/version1/cla.json` and covers all your future contributions, so you will not be asked again.

If you contribute as part of your job, make sure your employer permits it.

## Building

See [BUILDING.md](BUILDING.md) for prerequisites and build options.

```powershell
.\build.ps1
```

## Pull requests

- Branch from `main` and keep each pull request focused on one change.
- Match the existing C++23 / Win32 style in `src/`; a `.clang-format` is provided.
- Every source file carries the GPL header from `COPYRIGHT.txt` in the repository root. New files need it too.
- Run `.\build.ps1` before submitting; it must build cleanly at `/W4` and the unit tests must pass. CI also builds with clang-cl.
- Describe what the change does and how you tested it. If it changes something visible, a before/after screenshot helps.
- User-visible changes should get a line in the changelog (see below).

## Changelog and release notes

**Your part:** if the change is visible to a user, add one line under the `## [Unreleased]` heading at the top of [docs/CHANGELOG.md](CHANGELOG.md), in the matching `### Added` / `### Changed` / `### Fixed` group (create the group if it is missing).

Keep it to one or two sentences: what changed, and why if that is not obvious. No symbol names, no file paths, no implementation detail. That belongs in the commit message. Do not add a version heading, do not edit already-released sections, and do not touch anything under `Distribute/`.

**Maintainer's part:** versioning, the dated release section, tagging, and publishing the GitHub Release.

If you are not sure a change is user-visible, add the line anyway; it is easier to drop one than to reconstruct it at release time.
