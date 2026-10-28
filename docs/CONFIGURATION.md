# Configuring DexPresenter

Most settings are in the Settings dialog: right-click the tray icon and choose Settings.

## Start with Windows

Start with Windows is off by default. Turn it on from the General tab of the Settings dialog and DexPresenter starts automatically when you sign in. Turning it off removes the autorun entry immediately.

## config.ini

Settings are stored in `%AppData%\DexPresenter\config.ini`. DexPresenter writes the file itself, so you rarely need to edit it. If you report a toolbar position, settings or persistence bug, attaching it helps.

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

An empty `screenshotDir` means the default folder, `%USERPROFILE%\Pictures\DexPresenter`.
