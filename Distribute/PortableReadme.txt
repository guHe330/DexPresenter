================================================================================
                                  DexPresenter
================================================================================

A lightweight presentation tool for Windows: zoom into your screen, draw
annotations over it, snip a screenshot, or put a full-screen countdown up
during a break. Everything is reachable from a small always-on-top toolbar,
so there are no hotkeys to memorize mid-talk.


================================================================================
GETTING STARTED (PORTABLE)
================================================================================

  1. Extract this zip anywhere you like -- a USB stick is fine.
  2. Run DexPresenter.exe.
  3. The toolbar appears at the top edge of your screen, and a DexPresenter
     icon appears in the system tray.
  4. Drag the toolbar's grip to move it. It remembers where you put it on
     each monitor.

There is nothing to install and no administrator rights are needed. To
uninstall, close DexPresenter and delete the folder (see WHAT IT WRITES
below for the two other places to clean up).


================================================================================
KEY INFORMATION
================================================================================

Status:         Stable
Technology:     Native C++23 (Win32 API)
Binary Size:    ~460 KB, no runtime dependencies (static CRT)
Platform:       Windows 11
License:        GPL-3.0
Source:         https://github.com/guHe330/DexPresenter


================================================================================
THE FOUR TOOLS
================================================================================

All four are launched from the toolbar. There are no global hotkeys to learn;
the shortcuts listed below apply inside a tool once it is running.

Snip
  Drag a rectangle over the screen. The selection goes to the clipboard and
  is saved as a PNG.

Draw
  Freezes the screen and lets you annotate it. Right-click anywhere for the
  tool panel: freehand, arrow, box, circle, obfuscate, eraser, eight colors
  and four pen widths. Each shape remembers its own color and width across
  sessions.
    Ctrl+Z    undo
    Ctrl+Y    redo
    Shift     hold while dragging for perfect squares and circles
    ESC       exit (the annotated screenshot is saved automatically)

  "Obfuscate" pixelates whatever you drag over -- useful for an email
  address or an account number that ended up on a slide.

Timer
  A full-screen countdown for breaks. Ten minutes by default; change it in
  Settings. ESC dismisses it.

Zoom
  Freezes the screen and magnifies it. Move the mouse to pan, scroll to
  change the zoom level, ESC to exit.


================================================================================
WHAT IT WRITES, AND WHERE
================================================================================

DexPresenter is ad-free, collects no telemetry, and makes no network calls
of any kind -- a CI check fails the build if networking code ever appears in
it. Nothing leaves your machine. It touches exactly three places:

  Settings     %AppData%\DexPresenter\config.ini
               Zoom level, timer duration, per-tool colors and widths, and
               the toolbar position on each monitor.

  Screenshots  %USERPROFILE%\Pictures\DexPresenter\
               Snips and annotated captures, timestamped. The folder is
               configurable on the General tab of Settings.

  Registry     Only if you switch on "Start with Windows" (off by default):
               a single entry under
               HKCU\Software\Microsoft\Windows\CurrentVersion\Run

Delete those and DexPresenter has left no trace.


================================================================================
VERIFYING THIS DOWNLOAD
================================================================================

DexPresenter is distributed without a commercial code-signing certificate,
so Windows SmartScreen may warn on first run and report an unknown
publisher. You do not have to take the download on trust.

Every release publishes a SHA-256 for this zip on its release page:

    Get-FileHash Portable_DexPresenter.zip -Algorithm SHA256

Both this package and the DexPresenter.exe inside it also carry a GitHub
build provenance attestation, which ties them to the exact workflow run and
commit that built them. With the GitHub CLI installed:

    gh attestation verify DexPresenter.exe --repo guHe330/DexPresenter

That is a supply-chain check, not a Windows signature -- it confirms where
the file came from, but does not remove the SmartScreen prompt.


================================================================================
HELP, BUGS, AND SOURCE
================================================================================

Bug reports are appreciated, and test reports from unusual display setups
even more so -- multiple monitors, mixed DPI scaling, projectors and docks
are the parts most likely to misbehave and the hardest to test in-house.

  Issues:   https://github.com/guHe330/DexPresenter/issues
  Manual:   https://github.com/guHe330/DexPresenter#readme
  Changes:  https://github.com/guHe330/DexPresenter/blob/main/docs/CHANGELOG.md
  Security: report privately via the repository's Security tab, not a
            public issue.

DexPresenter is free software under the GNU General Public License v3 (see
LICENSE). You may use, study, modify and redistribute it under those terms.

The complete corresponding source is in this package, as
DexPresenter_Sources.zip -- the exact tree this DexPresenter.exe was built
from, not a snapshot of whatever main happens to hold. Unzip it and run
build.ps1, or open it with any CMake-aware IDE. The live repository and full
history are at the GitHub link above.

The application icon and image assets are NOT covered by the GPLv3 and
remain proprietary to Gunter Heiss -- if you fork or redistribute
DexPresenter, replace them with your own artwork.

Third-party components are listed in THIRD_PARTY_NOTICES.txt.
