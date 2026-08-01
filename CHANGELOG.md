# Changelog

All notable changes to DexPresenter are documented here.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

## [1.0.0] — not yet released

Initial public release.

### Fixed
- On displays with scaling (125 %+, typical on laptops), the Draw/Zoom/Snip overlays captured only part of the screen and appeared zoomed in, with strokes offset from the cursor. The app now declares per-monitor DPI awareness, so captures match the real screen resolution and rendering is crisp on scaled displays.

### Added
- Zoom tool: capture and zoom into the screen, pan with the mouse, scroll to change zoom level (`Ctrl+1`).
- Draw tool: annotate a screen capture with freehand, smooth curve, arrow, box, circle, obfuscate (pixelate), and eraser modes; per-shape color/width memory; undo/redo; auto-saves the annotated screenshot on exit (`Ctrl+2`).
- Timer tool: full-screen break countdown (`Ctrl+3`).
- Snip tool: rectangular screen capture to clipboard and PNG (`Ctrl+6`).
- Always-on-top draggable toolbar, system tray integration, tabbed settings dialog.
- Toolbar dragging is confined to the visible screen: it stops at the monitor edges instead of going partly off-screen, and hands over to the neighbouring monitor as the cursor crosses the boundary.
- The toolbar rubber-bands to the work-area edges while dragging, so resting it flush on top of the taskbar is pixel-perfect; drag more than a few pixels further and it releases and floats over the taskbar.
- The toolbar remembers where it was placed on each monitor, across restarts. When the display layout changes it stays on its own monitor if that monitor survived, and otherwise returns to the primary monitor at the position it was last placed there. Placements are stored per display device under `[toolbar.monitors]` in `config.ini`, as an offset from that monitor's corner, so rearranging screens does not invalidate them.
- Optional "Start with Windows" (off by default, owned entirely by the app).
- Portable single-exe distribution and Inno Setup installer including the complete corresponding source.
- Build system: C++23, `/W4` warnings, CMake presets, Ninja builds, and support for both MSVC (release compiler) and clang-cl (CI-verified).
