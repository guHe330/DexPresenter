# Building DexPresenter

You need Visual Studio 2022 with the C++ workload and CMake 3.21+. The build uses Ninja when it is available (Visual Studio bundles it) and falls back to NMake.

```powershell
.\build.ps1                    # Release build + tests (MSVC)
.\build.ps1 -Config Debug      # Debug build
.\build.ps1 -Clean             # Clean rebuild
.\build.ps1 -BuildInstaller    # Build Inno Setup installer
.\build.ps1 -SkipTests         # Skip tests
.\build.ps1 -Clang             # Build with clang-cl instead of MSVC (output in build-clang\)
```

The code is C++23 and compiles clean at `/W4` on both compilers. Please keep it that way in pull requests.

## Compilers

All official binaries are built with MSVC, with a static CRT and no runtime dependencies. CI also builds with clang-cl for its extra warnings. To build with clang locally, install the "C++ Clang tools for Windows" component in the Visual Studio Installer (or LLVM from [llvm.org](https://llvm.org)), then run `.\build.ps1 -Clang`.

## CMake presets

To use CMake directly, or an IDE that reads [CMakePresets.json](../CMakePresets.json), run these from a Developer PowerShell for VS 2022:

```powershell
cmake --preset msvc && cmake --build --preset msvc      # MSVC Release → build\
cmake --preset clang && cmake --build --preset clang    # clang-cl Release → build-clang\
```

## Tests

`build.ps1` runs the unit tests (INI round trips, config defaults, time formatting) at the end of every build. Pass `-SkipTests` to skip them during quick iteration, or run `build\DexPresenterTests.exe` by hand.

## Color scheme

One palette covers both the interface and the icons, so the toolbar, dialogs and icons match. It is defined once in [src/Constants.h](../src/Constants.h) as `namespace Theme`. Use these values for new UI and for new icons.

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
