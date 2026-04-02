# DexPresenter

DexPresenter is a lightweight presentation tool: under 500 KB, no telemetry, no dependencies.

- Zoom into your screen during presentations
- Draw arrows, highlights, and shapes
- Snip screenshots instantly
- Show a fullscreen timer during breaks
- All accessible from a tiny toolbar (no hotkeys required)

<!-- TODO DexPresenter in action [ GIF: activate draw → draw arrow → zoom → exit ] -->

DexPresenter emphasizes discoverability: every drawing tool and setting is accessible via a right-click panel, so presenters don’t need to memorize hotkeys during a talk.

It is inspired by [ZoomIt](https://learn.microsoft.com/en-us/windows/powertoys/zoomit) and provides screen zoom, drawing annotations, a break timer, and screen snipping, all accessible from a small always-on-top toolbar or configurable hotkeys.

> **A personal project, shared openly.** DexPresenter is built and maintained by a single developer as a passion project. It's not backed by a company or a team, just one person who kept forgetting ZoomIt's hotkeys.

> **Privacy & Trust.** DexPresenter is ad-free, collects no telemetry or user data, and never communicates with any server. Everything the application needs lives in two places on your PC: `Program Files\DexPresenter` and `%AppData%\DexPresenter`. That's it, nothing leaves your machine.

> **Alpha** - This project is in early development. Expect breaking changes, missing features, and rough edges.

## Key Information

- **Status**: Alpha
- **Version**: Native C++ (Win32 API)
- **Executable Size**: ~480 KB
- **Platform**: Windows 11
- **License**: GPLv3

## Features

<!-- TODO screenshot: toolbar docked at top edge of screen, showing S/D/T/Z buttons and close button -->

### Tools

| Tool  | Default Hotkey | Description                                                                                                        |
| ----- | -------------- | ------------------------------------------------------------------------------------------------------------------ |
| Snip  | `Ctrl+6`       | Rectangle screen selection. Saves to clipboard and PNG file.                                                       |
| Draw  | `Ctrl+2`       | Capture screen, draw annotations. Right-click for tool/color/width panel. Auto-saves annotated screenshot on exit. |
| Timer | `Ctrl+3`       | Full-screen countdown timer (default 10 minutes). ESC to dismiss.                                                  |
| Zoom  | `Ctrl+1`       | Capture screen, overlay zoomed view. Mouse to pan, scroll to zoom, ESC to exit.                                    |

<!-- TODO gif: zoom tool — activating zoom, panning around, scrolling to change zoom level -->
<!-- TODO screenshot: timer tool — full-screen countdown overlay -->
<!-- TODO gif: snip tool — dragging a rectangle selection, showing the saved result -->

### Drawing Tools

<!-- TODO gif: draw mode in action — freehand annotation over a slide, switching colors, then undo/redo -->

- **Shape Modes**: Freehand (F), Smooth (S), Arrow (A), Box (B), Circle (C), Obfuscate (V), Eraser (X)
- **Per-Tool Settings**: Each shape mode remembers its own color and width across sessions
- **Color Palette**: 8 colors accessible via right-click panel
- **Width Presets**: 4 pen widths accessible via right-click panel
- **Quick Color Keys**: R (red), G (green), Y (yellow), O (orange), P (pink), W (white), K (black)
- **Undo**: Ctrl+Z to undo last action (drawing or erasing)
- **Redo**: Ctrl+Y to redo last undone action
- **Obfuscate**: Drag to pixelate screen regions, hides sensitive information
<!-- TODO gif: obfuscate in action — dragging over an email address or password to pixelate it -->
- **Eraser**: Drag to erase elements, fully undoable
- **Clear All**: E to clear all elements
- **Shift-Constrain**: Hold Shift for perfect squares (Box) or circles (Circle)

<!-- TODO screenshot: right-click panel showing shape modes, color palette, and width presets -->

### User Interface

- **Always-On-Top Toolbar**: Compact floating toolbar with tool buttons (S, D, T, Z) and close button
- **Dark Theme**: Dark grey + pink color scheme with anti-aliased GDI+ rendering
- **Tray Integration**: System tray icon with right-click menu (Show Toolbar, Settings, Exit)
- **Draggable Toolbar**: Click and drag the toolbar to reposition; position is persisted
- **Settings Dialog**: Tabbed PropertySheet for configuring hotkeys, zoom level, and timer duration

<!-- TODO screenshot: settings dialog showing the hotkey/zoom/timer tabs -->
<!-- TODO screenshot: system tray icon with right-click context menu -->

### Screenshots

- **Auto-Save**: Draw tool saves annotated screenshots on exit, Snip tool saves on capture
- **Default Directory**: `%USERPROFILE%\Pictures\DexPresenter`
- **Configurable**: Screenshot directory can be changed per-tool in settings
- **Timestamped**: Files named `DexPresenter_Draw_2026-03-05T14-30-45.png`

### Performance and Efficiency

- **Native Code**: Written in C++ using Win32 API for minimal resource usage and zero runtime overhead
- **Small Footprint**: ~480 KB standalone executable with no external dependencies (static CRT)
- **Instant Startup**: No JIT compilation or managed runtime delays

## Installation

### Installer (recommended)

Download and run the provided installer from the Releases page. Because DexPresenter is currently distributed without a commercial code-signing certificate, Windows SmartScreen may show a warning on first run. Click **More info** then **Run anyway** to proceed.

The installer includes the complete source code (`DexPresenter_Sources.zip`) in the installation directory, allowing you to inspect, modify, or rebuild the application in compliance with the GPLv3 license.

### Manual Installation

1.  Download `DexPresenter.exe` from the latest release.
2.  Place it anywhere (e.g. `C:\Program Files\DexPresenter\`).
3.  Run `DexPresenter.exe`. The toolbar will appear at the top edge of your screen.

## Configuration

Settings are stored as an INI file at `%AppData%\DexPresenter\config.ini`.

```ini
[zoom]
hotkey_modifiers = 2
hotkey_vkey = 49
defaultZoomLevel = 2.00

[draw]
hotkey_modifiers = 2
hotkey_vkey = 50
freehand_colorR = 255
freehand_colorG = 0
freehand_colorB = 0
freehand_width = 3
arrow_colorR = 232
arrow_colorG = 96
arrow_colorB = 140
arrow_width = 3
lastMode = 0
screenshotDir =

[timer]
hotkey_modifiers = 2
hotkey_vkey = 51
durationSeconds = 600

[snip]
hotkey_modifiers = 2
hotkey_vkey = 54
screenshotDir =

[toolbar]
posX = -1
posY = 0
visible = 1

[general]
startWithWindows = 0
```

## Color Scheme

DexPresenter uses a dark grey + pink theme:

| Role           | Color       | Hex       |
| -------------- | ----------- | --------- |
| Background     | Near-black  | `#1A1A1E` |
| Surface        | Dark grey   | `#2A2A30` |
| Surface hover  | Medium grey | `#3A3A42` |
| Border         | Subtle grey | `#4A4A52` |
| Text primary   | Light grey  | `#E8E8EC` |
| Text secondary | Muted grey  | `#9898A0` |
| Accent         | Pink        | `#E8608C` |
| Accent hover   | Light pink  | `#F07098` |
| Accent dim     | Dark pink   | `#B84870` |
| Danger         | Red         | `#C85050` |

## Testing

Unit tests (INI round-trips, config defaults, time formatting) run automatically at the end of `build.ps1`. To skip them during quick iteration: `build.ps1 -SkipTests`. To run manually: `build\DexPresenterTests.exe`.

## Building from Source

Requires Visual Studio 2022 with C++ workload and CMake 3.15+.

```powershell
.\build.ps1                    # Release build + tests
.\build.ps1 -Config Debug      # Debug build
.\build.ps1 -Clean             # Clean rebuild
.\build.ps1 -BuildInstaller    # Build Inno Setup installer
.\build.ps1 -SkipTests         # Skip tests
```

## License

This project is licensed under the GPLv3 License - see the [LICENSE](LICENSE) file for details.
