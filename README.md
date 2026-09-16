# DexPresenter

[![CI](https://github.com/guHe330/DexPresenter/actions/workflows/ci.yml/badge.svg)](https://github.com/guHe330/DexPresenter/actions/workflows/ci.yml)
[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-blue.svg)](LICENSE)
[![Latest release](https://img.shields.io/github/v/release/guHe330/DexPresenter?include_prereleases)](https://github.com/guHe330/DexPresenter/releases)
[![Platform](https://img.shields.io/badge/platform-Windows%2011-0078d4.svg)](#key-information)

DexPresenter is a lightweight presentation tool: under 500 KB, no telemetry, no dependencies.

- Zoom into your screen during presentations
- Draw arrows, highlights, and shapes
- Snip screenshots instantly
- Show a fullscreen timer during breaks
- All accessible from a tiny toolbar — no hotkeys to memorize

<!-- TODO DexPresenter in action [ GIF: activate draw → draw arrow → zoom → exit ] -->

DexPresenter emphasizes discoverability: every tool and setting is accessible from the toolbar or a right-click panel, so presenters don't need to memorize anything during a talk.

It is inspired by [ZoomIt](https://learn.microsoft.com/en-us/windows/powertoys/zoomit) and provides screen zoom, drawing annotations, a break timer, and screen snipping, all from a small always-on-top toolbar.

> **A personal project, shared openly.** DexPresenter is built and maintained by a single developer as a passion project. It's not backed by a company or a team, just one person who kept forgetting ZoomIt's hotkeys.

> **Privacy & Trust.** DexPresenter is ad-free, collects no telemetry or user data, and never communicates with any server. Everything the application needs lives in two places on your PC: `Program Files\DexPresenter` and `%AppData%\DexPresenter`. That's it, nothing leaves your machine. You don't have to take that on trust: [a CI guard](.github/workflows/security-guards.yml) fails the build if any networking API ever appears in the shipped code.

> **Alpha** - This project is in early development. Expect breaking changes, missing features, and rough edges.

## Key Information

- **Status**: Alpha
- **Technology**: Native C++23 (Win32 API)
- **Executable Size**: ~460 KB
- **Platform**: Windows 11
- **License**: GPLv3

## Features

<!-- TODO screenshot: toolbar docked at top edge of screen, showing the Snip/Draw/Timer/Zoom icon buttons and close button -->

### Tools

| Tool  | Description                                                                                                        |
| ----- | ------------------------------------------------------------------------------------------------------------------ |
| Snip  | Rectangle screen selection. Saves to clipboard and PNG file.                                                       |
| Draw  | Capture screen, draw annotations. Right-click for tool/color/width panel. Auto-saves annotated screenshot on exit. |
| Timer | Full-screen countdown timer (default 10 minutes). ESC to dismiss.                                                  |
| Zoom  | Capture screen, overlay zoomed view. Mouse to pan, scroll to zoom, ESC to exit.                                    |

<!-- TODO gif: zoom tool — activating zoom, panning around, scrolling to change zoom level -->
<!-- TODO screenshot: timer tool — full-screen countdown overlay -->
<!-- TODO gif: snip tool — dragging a rectangle selection, showing the saved result -->

### Drawing Tools

<!-- TODO gif: draw mode in action — freehand annotation over a slide, switching colors, then undo/redo -->

- **Shape Modes**: Freehand, Arrow, Box, Circle, Obfuscate, Eraser — all accessible via right-click panel
- **Per-Tool Settings**: Each shape mode remembers its own color and width across sessions
- **Color Palette**: 8 colors accessible via right-click panel
- **Width Presets**: 4 pen widths accessible via right-click panel
- **Undo**: Ctrl+Z to undo last action (drawing or erasing)
- **Redo**: Ctrl+Y to redo last undone action
- **Obfuscate**: Drag to pixelate screen regions, hides sensitive information
<!-- TODO gif: obfuscate in action — dragging over an email address or password to pixelate it -->
- **Eraser**: Drag to erase elements, fully undoable
- **Clear All**: accessible via right-click panel
- **Shift-Constrain**: Hold Shift for perfect squares (Box) or circles (Circle)

<!-- TODO screenshot: right-click panel showing shape modes, color palette, and width presets -->

### User Interface

- **Always-On-Top Toolbar**: Compact floating toolbar with Snip/Draw/Timer/Zoom icon buttons and a close button
- **Dark Theme**: Dark grey + pink color scheme with anti-aliased GDI+ rendering
- **Tray Integration**: System tray icon with right-click menu (Show Toolbar, Settings, Exit)
- **Draggable Toolbar**: Click and drag the grip to reposition; the toolbar always stays fully on screen, snaps flush against the taskbar (drag further to float over it), moves freely between monitors, and remembers where you put it on each one. Position is persisted
- **Settings Dialog**: Tabbed PropertySheet for configuring zoom level and timer duration

<!-- TODO screenshot: settings dialog showing the zoom/timer tabs -->
<!-- TODO screenshot: system tray icon with right-click context menu -->

### Screenshots

- **Auto-Save**: Draw tool saves annotated screenshots on exit, Snip tool saves on capture
- **Default Directory**: `%USERPROFILE%\Pictures\DexPresenter`
- **Configurable**: Screenshot folder can be changed on the General tab of settings
- **Timestamped**: Files named `DexPresenter_Draw_2026-03-05T14-30-45.png`

### Performance and Efficiency

- **Native Code**: Written in C++ using Win32 API for minimal resource usage and zero runtime overhead
- **Small Footprint**: ~460 KB standalone executable with no external dependencies (static CRT), including all embedded icon resources
- **Instant Startup**: No JIT compilation or managed runtime delays

## Installation

### Installer (recommended)

Download and run the provided installer from the Releases page. Because DexPresenter is currently distributed without a commercial code-signing certificate, Windows SmartScreen may show a warning on first run. Click **More info** then **Run anyway** to proceed.

The installer includes the complete source code (`DexPresenter_Sources.zip`) in the installation directory, allowing you to inspect, modify, or rebuild the application in compliance with the GPLv3 license.

### Manual Installation

1.  Download `DexPresenter.exe` from the latest release.
2.  Place it anywhere (e.g. `C:\Program Files\DexPresenter\`).
3.  Run `DexPresenter.exe`. The toolbar will appear at the top edge of your screen.

### Verifying your download

Every release page publishes a SHA-256 for each artifact:

```powershell
Get-FileHash DexPresenter_1.0.0_Setup.exe -Algorithm SHA256
```

Both artifacts also carry a [GitHub build provenance attestation](https://docs.github.com/actions/security-guides/using-artifact-attestations-to-establish-provenance-for-builds), which ties them to the exact workflow run and commit that produced them. With the [GitHub CLI](https://cli.github.com/):

```powershell
gh attestation verify DexPresenter_1.0.0_Setup.exe --repo guHe330/DexPresenter
```

That is a supply-chain check, not a Windows signature — it does not remove the SmartScreen prompt above.

## Configuration

Settings are stored as an INI file at `%AppData%\DexPresenter\config.ini`.

```ini
[zoom]
defaultZoomLevel = 2.00

[draw]
freehand_colorR = 255
freehand_colorG = 0
freehand_colorB = 0
freehand_width = 3
arrow_colorR = 232
arrow_colorG = 96
arrow_colorB = 140
arrow_width = 3
lastMode = 0

[timer]
durationSeconds = 600

[toolbar]
visible = 1
monitor = \\.\DISPLAY1

; Where the toolbar was last placed on each monitor, as "x,y" measured from that
; monitor's top-left corner. Written automatically as you drag it around.
[toolbar.monitors]
\\.\DISPLAY1 = 1512,1032
\\.\DISPLAY2 = 40,8

[general]
startWithWindows = 0
screenshotDir =
```

**Start with Windows** is off by default. Toggle it from the **General** tab of the Settings dialog (right-click the tray icon → Settings); enabling it launches DexPresenter automatically at sign-in, and disabling it removes the autorun entry immediately. You can change it at any time.

## Color Scheme

DexPresenter uses a dark plum + pink theme. A single palette covers both the
interface and the artwork, so the toolbar, dialogs, and icons match exactly.
It is defined once in [src/Constants.h](src/Constants.h) as `namespace Theme`;
use these values for new UI *and* for new icons.

| Role           | Color       | Hex       | Used for                                          |
| -------------- | ----------- | --------- | ------------------------------------------------- |
| Background     | Near-black  | `#141218` | Toolbar and overlay backdrops; icon page backdrop |
| Surface        | Dark plum   | `#211F26` | Elevated panels and popouts; icon tile interior   |
| Surface hover  | Plum grey   | `#3B3940` | Hover states                                      |
| Border         | Muted grey  | `#565359` | Subtle borders, separators, the toolbar grip      |
| Text primary   | Off-white   | `#F2EEF4` | Main text, button labels, icon glyph strokes      |
| Text secondary | Muted plum  | `#99969C` | Hints and secondary labels                        |
| Accent         | Pink        | `#E64A81` | Buttons, highlights, icon tile borders            |
| Accent hover   | Light pink  | `#F7659F` | Hover state, icon highlight details               |
| Accent dim     | Dark maroon | `#662E46` | Pressed/active; the dim ring behind the icon dot  |
| Danger         | Red         | `#E85555` | Close button, destructive actions                 |

## Testing

Unit tests (INI round-trips, config defaults, time formatting) run automatically at the end of `build.ps1`. To skip them during quick iteration: `build.ps1 -SkipTests`. To run manually: `build\DexPresenterTests.exe`.

## Building from Source

Requires Visual Studio 2022 with the C++ workload and CMake 3.21+. The build uses Ninja when available (bundled with VS) and falls back to NMake.

```powershell
.\build.ps1                    # Release build + tests (MSVC)
.\build.ps1 -Config Debug      # Debug build
.\build.ps1 -Clean             # Clean rebuild
.\build.ps1 -BuildInstaller    # Build Inno Setup installer
.\build.ps1 -SkipTests         # Skip tests
.\build.ps1 -Clang             # Build with clang-cl instead of MSVC (output in build-clang\)
```

**Compilers.** MSVC is the release compiler — all official binaries are built with it (static CRT, no runtime dependencies). clang-cl is also supported and built in CI for extra warning coverage. To build with clang locally, install the **"C++ Clang tools for Windows"** component via the Visual Studio Installer (or LLVM from [llvm.org](https://llvm.org)), then run `.\build.ps1 -Clang`.

**CMake presets.** If you prefer working with CMake directly (or an IDE that understands [CMakePresets.json](CMakePresets.json)), run from a *Developer PowerShell for VS 2022*:

```powershell
cmake --preset msvc && cmake --build --preset msvc      # MSVC Release → build\
cmake --preset clang && cmake --build --preset clang    # clang-cl Release → build-clang\
```

The code is C++23 and compiles clean at `/W4` on both compilers; please keep it that way in PRs.

## Contributing

Bug reports and focused pull requests are welcome — please read [CONTRIBUTING.md](docs/CONTRIBUTING.md) first; it sets honest expectations for a one-person project.

**Testing is the most useful contribution right now.** Development covers one desk. Multiple monitors, mixed DPI scaling, projectors and docks are where DexPresenter is most likely to misbehave and hardest to test in-house — the [multi-monitor / DPI test report](https://github.com/guHe330/DexPresenter/issues/new?template=multi_monitor_report.yml) form takes five minutes, and "it all worked" is worth posting too.

Code contributions are covered by a three-line [CLA](docs/CLA.md): your contribution is under the same GPLv3 as the project, and you keep your copyright. Signing is automated on your first pull request.

For security issues see [SECURITY.md](SECURITY.md), for support see [SUPPORT.md](docs/SUPPORT.md), and for release history see [CHANGELOG.md](docs/CHANGELOG.md).

## License

This project is licensed under the GPLv3 License - see the [LICENSE](LICENSE) file for details.

The application **icon and image assets** (`resources/DexPresenter.ico` and `resources/icons/`) are **not** covered by the GPLv3; they remain proprietary to Gunter Heiss under separate terms — see [resources/ICON_LICENSE.txt](resources/ICON_LICENSE.txt). If you fork or redistribute DexPresenter, replace them with your own artwork.

Third-party components and their licenses are listed in [THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt).

For privacy details, see [PRIVACY.md](docs/PRIVACY.md).
