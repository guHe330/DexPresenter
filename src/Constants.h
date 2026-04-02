#pragma once
#include <Windows.h>
#include <gdiplus.h>

// ============================================================================
// DexPresenter Color Scheme — Dark Grey + Pink
// ============================================================================
//
//   Background:     #1A1A1E  (26, 26, 30)    — near-black with slight blue
//   Surface:        #2A2A30  (42, 42, 48)    — elevated panels, popouts
//   Surface hover:  #3A3A42  (58, 58, 66)    — hover states
//   Border:         #4A4A52  (74, 74, 82)    — subtle borders
//   Text primary:   #E8E8EC  (232,232,236)   — main text, button labels
//   Text secondary: #9898A0  (152,152,160)   — hints, secondary labels
//   Accent:         #E8608C  (232, 96,140)   — pink accent (buttons, highlights)
//   Accent hover:   #F07098  (240,112,152)   — pink hover
//   Accent dim:     #B84870  (184, 72,112)   — pink pressed/active
//   Danger:         #C85050  (200, 80, 80)   — close button, destructive
//
namespace Theme {
    // Backgrounds
    constexpr BYTE BG_R = 26,  BG_G = 26,  BG_B = 30;
    constexpr BYTE SF_R = 42,  SF_G = 42,  SF_B = 48;   // Surface
    constexpr BYTE SH_R = 58,  SH_G = 58,  SH_B = 66;   // Surface hover
    constexpr BYTE BD_R = 74,  BD_G = 74,  BD_B = 82;   // Border

    // Text
    constexpr BYTE TX_R = 232, TX_G = 232, TX_B = 236;  // Primary text
    constexpr BYTE TS_R = 152, TS_G = 152, TS_B = 160;  // Secondary text

    // Pink accent
    constexpr BYTE AC_R = 232, AC_G = 96,  AC_B = 140;  // Accent
    constexpr BYTE AH_R = 240, AH_G = 112, AH_B = 152;  // Accent hover
    constexpr BYTE AD_R = 184, AD_G = 72,  AD_B = 112;  // Accent dim

    // Danger (close)
    constexpr BYTE DN_R = 200, DN_G = 80,  DN_B = 80;

    // GDI+ helpers
    inline Gdiplus::Color Background(BYTE a = 240) { return Gdiplus::Color(a, BG_R, BG_G, BG_B); }
    inline Gdiplus::Color Surface(BYTE a = 255)     { return Gdiplus::Color(a, SF_R, SF_G, SF_B); }
    inline Gdiplus::Color SurfaceHover(BYTE a = 255){ return Gdiplus::Color(a, SH_R, SH_G, SH_B); }
    inline Gdiplus::Color Border(BYTE a = 255)      { return Gdiplus::Color(a, BD_R, BD_G, BD_B); }
    inline Gdiplus::Color Text(BYTE a = 255)        { return Gdiplus::Color(a, TX_R, TX_G, TX_B); }
    inline Gdiplus::Color TextSecondary(BYTE a=255) { return Gdiplus::Color(a, TS_R, TS_G, TS_B); }
    inline Gdiplus::Color Accent(BYTE a = 255)      { return Gdiplus::Color(a, AC_R, AC_G, AC_B); }
    inline Gdiplus::Color AccentHover(BYTE a = 255) { return Gdiplus::Color(a, AH_R, AH_G, AH_B); }
    inline Gdiplus::Color AccentDim(BYTE a = 255)   { return Gdiplus::Color(a, AD_R, AD_G, AD_B); }
    inline Gdiplus::Color Danger(BYTE a = 255)      { return Gdiplus::Color(a, DN_R, DN_G, DN_B); }
}

// Application identity
constexpr const wchar_t* APP_NAME          = L"DexPresenter";
constexpr const wchar_t* APP_WINDOW_CLASS  = L"DexPresenterMainWindow";
constexpr const wchar_t* APP_MUTEX_NAME    = L"Global\\DexPresenter_SingleInstance";

// Hotkey IDs (for RegisterHotKey)
constexpr int HOTKEY_ZOOM  = 1;
constexpr int HOTKEY_DRAW  = 2;
constexpr int HOTKEY_TIMER = 3;
constexpr int HOTKEY_SNIP  = 6;

// Custom window messages
constexpr UINT WM_TRAYICON       = WM_USER + 1;
constexpr UINT WM_ACTIVATE_ZOOM  = WM_USER + 10;
constexpr UINT WM_ACTIVATE_DRAW  = WM_USER + 11;
constexpr UINT WM_ACTIVATE_TIMER = WM_USER + 12;
constexpr UINT WM_ACTIVATE_SNIP  = WM_USER + 13;
constexpr UINT WM_TOOL_FINISHED  = WM_USER + 20;

// Zoom defaults
constexpr float ZOOM_DEFAULT_LEVEL = 2.0f;
constexpr float ZOOM_MIN_LEVEL     = 1.1f;
constexpr float ZOOM_MAX_LEVEL     = 10.0f;
constexpr float ZOOM_SCROLL_STEP   = 0.2f;

// Draw defaults
constexpr int DRAW_DEFAULT_PEN_WIDTH = 3;
constexpr int DRAW_MIN_PEN_WIDTH     = 1;
constexpr int DRAW_MAX_PEN_WIDTH     = 20;

// Obfuscate tool
constexpr int OBFUSCATE_PIXEL_SIZE   = 15;  // Mosaic block size; tune for readability vs. coverage

// Snip magnifier
constexpr int MAG_SIZE       = 130;   // Magnifier panel width/height
constexpr int MAG_ZOOM       = 6;     // Zoom factor
constexpr int MAG_OFFSET     = 20;    // Distance from cursor
constexpr int MAG_CORNER_RAD = 10;    // Rounded corner radius
constexpr int MAG_SRC_SIZE   = MAG_SIZE / MAG_ZOOM; // Source pixel area

// Timer defaults
constexpr int TIMER_DEFAULT_SECONDS = 600; // 10 minutes

// Toolbar dimensions
constexpr int TOOLBAR_BUTTON_SIZE = 36;
constexpr int TOOLBAR_PADDING     = 4;
constexpr int TOOLBAR_HEIGHT      = TOOLBAR_BUTTON_SIZE + 2 * TOOLBAR_PADDING;
constexpr int TOOLBAR_NUM_BUTTONS = 5; // 4 tools + close
constexpr int TOOLBAR_WIDTH       = TOOLBAR_NUM_BUTTONS * (TOOLBAR_BUTTON_SIZE + TOOLBAR_PADDING) + TOOLBAR_PADDING;
