# Changelog

All notable changes to DexPresenter are documented here.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

## [1.0.0] - 2026-09-16

Initial public release.

### Fixed
- On displays with scaling (125 %+, typical on laptops), the Draw/Zoom/Snip overlays captured only part of the screen and appeared zoomed in, with strokes offset from the cursor. The app now declares per-monitor DPI awareness, so captures match the real screen resolution and rendering is crisp on scaled displays.

### Added
- Zoom tool: capture and zoom into the screen, pan with the mouse, scroll to change zoom level.
- Draw tool: annotate a screen capture with freehand, smooth curve, arrow, box, circle, obfuscate (pixelate), and eraser modes; per-shape color/width memory; undo/redo; auto-saves the annotated screenshot on exit.
- Timer tool: full-screen break countdown.
- Snip tool: rectangular screen capture to clipboard and PNG.
- Always-on-top draggable toolbar, system tray integration, tabbed settings dialog.
- Toolbar dragging is confined to the visible screen: it stops at the monitor edges instead of going partly off-screen, and hands over to the neighbouring monitor as the cursor crosses the boundary.
- The toolbar rubber-bands to the work-area edges while dragging, so resting it flush on top of the taskbar is pixel-perfect; drag more than a few pixels further and it releases and floats over the taskbar.
- The toolbar remembers where it was placed on each monitor, across restarts. When the display layout changes it stays on its own monitor if that monitor survived, and otherwise returns to the primary monitor at the position it was last placed there. Placements are stored per display device under `[toolbar.monitors]` in `config.ini`, as an offset from that monitor's corner, so rearranging screens does not invalidate them.
- Optional "Start with Windows" (off by default, owned entirely by the app).
- Portable single-exe distribution and Inno Setup installer. Both carry the complete corresponding source, which is also published as its own release asset.
- Build system: C++23, `/W4` warnings, CMake presets, Ninja builds, and support for both MSVC (release compiler) and clang-cl (CI-verified).
- Every release artifact is published with a SHA-256 checksum and a GitHub build provenance attestation, so a download can be verified before it is run. The binaries are unsigned, so SmartScreen still warns; this at least makes the download checkable instead of something you have to take on trust. Verify with `Get-FileHash`, or `gh attestation verify <file> --repo guHe330/DexPresenter`.
- The no-network promise is now enforced by CI rather than only stated: a guard fails the build if any networking API appears in the shipped code, or if a networking library is linked.
- Contributing is covered by a three-line, inbound-equals-outbound GPLv3 CLA: contributions are licensed under the same GPLv3 as the project and contributors keep their copyright. Signing is automated on the pull request.
- A multi-monitor and DPI test report template, for the display setups that cannot be reproduced in-house.
