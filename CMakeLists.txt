# 3.21+ for CMakePresets.json v3 and C++23 support
cmake_minimum_required(VERSION 3.21)
project(DexPresenter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# No C++ modules in this codebase; skip the dependency-scanning pass that
# newer CMake enables by default for C++20+.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# High warning level for our own targets (MSVC and clang-cl); third-party
# code fetched below (googletest) is left at its own defaults.
set(DEXPRESENTER_WARNINGS /W4)

# Static CRT -- no vcredist needed
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(DexPresenter WIN32
    src/main.cpp
    src/App.cpp
    src/Toolbar.cpp
    src/OverlayWindow.cpp
    src/ZoomTool.cpp
    src/DrawTool.cpp
    src/TimerTool.cpp
    src/SnipTool.cpp
    src/Config.cpp
    src/TrayIcon.cpp
    src/SaveUtils.cpp
    src/SaveToast.cpp
    src/SettingsDialog.cpp
    resources/DexPresenter.rc
)

target_include_directories(DexPresenter PRIVATE src)
target_compile_definitions(DexPresenter PRIVATE UNICODE _UNICODE NOMINMAX)
target_compile_options(DexPresenter PRIVATE ${DEXPRESENTER_WARNINGS})

target_link_libraries(DexPresenter PRIVATE
    user32
    gdi32
    shell32
    ole32
    comctl32
    comdlg32
    gdiplus
    shlwapi
    msimg32
)

set_target_properties(DexPresenter PROPERTIES WIN32_EXECUTABLE TRUE)

# --- Unit Tests (Google Test via FetchContent) ---
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(DexPresenterTests
    tests/test_config.cpp
    tests/test_timer.cpp
    src/Config.cpp
)
target_include_directories(DexPresenterTests PRIVATE src)
target_compile_options(DexPresenterTests PRIVATE ${DEXPRESENTER_WARNINGS})
target_link_libraries(DexPresenterTests PRIVATE GTest::gtest_main shell32 shlwapi)
target_compile_definitions(DexPresenterTests PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
set_property(TARGET DexPresenterTests PROPERTY MSVC_RUNTIME_LIBRARY
    "MultiThreaded$<$<CONFIG:Debug>:Debug>")

enable_testing()
add_test(NAME DexPresenterTests COMMAND DexPresenterTests)
