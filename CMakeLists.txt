cmake_minimum_required(VERSION 3.15)
project(DexPresenter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    src/SettingsDialog.cpp
    resources/DexPresenter.rc
)

target_include_directories(DexPresenter PRIVATE src)
target_compile_definitions(DexPresenter PRIVATE UNICODE _UNICODE NOMINMAX)

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
target_link_libraries(DexPresenterTests PRIVATE GTest::gtest_main shell32 shlwapi)
target_compile_definitions(DexPresenterTests PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
set_property(TARGET DexPresenterTests PROPERTY MSVC_RUNTIME_LIBRARY
    "MultiThreaded$<$<CONFIG:Debug>:Debug>")

enable_testing()
add_test(NAME DexPresenterTests COMMAND DexPresenterTests)
