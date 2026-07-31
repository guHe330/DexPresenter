/**
 * DexPresenter - a free and open source Windows presentation utility with drawing tools
 * Copyright (C) 2026 Gunter Heiss
 *
 * For more information see: https://dexpresenter.com
 * The DexPresenter project is hosted on GitHub: https://github.com/guHe330/DexPresenter
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <Windows.h>
#include <gdiplus.h>

// ============================================================================
// DexPresenter Color Scheme — Dark Plum + Pink
// ============================================================================
//
// One palette for everything: the values below are the same ones the icon
// artwork in resources/ is drawn from, so the UI and the icons match exactly.
// Roles the icon set does not name are taken from colors already present in
// the artwork (marked "from art") rather than invented alongside it.
//
//   Background:     #141218  (20, 18, 24)    — near-black plum
//   Surface:        #211F26  (33, 31, 38)    — elevated panels, icon tile fill
//   Surface hover:  #3B3940  (59, 57, 64)    — hover states          (from art)
//   Border:         #565359  (86, 83, 89)    — subtle borders        (from art)
//   Text primary:   #F2EEF4  (242,238,244)   — main text, glyph white
//   Text secondary: #99969C  (153,150,156)   — hints; primary text mixed 60%
//                                              into the background
//   Accent:         #E64A81  (230, 74,129)   — pink accent (buttons, highlights)
//   Accent hover:   #F7659F  (247,101,159)   — pink hover
//   Accent dim:     #662E46  (102, 46, 70)   — pressed/active; the dim ring
//                                              behind the icon dot     (from art)
//   Danger:         #E85555  (232, 85, 85)   — close button, destructive
//
namespace Theme {
    // Backgrounds
    constexpr BYTE BG_R = 20,  BG_G = 18,  BG_B = 24;
    constexpr BYTE SF_R = 33,  SF_G = 31,  SF_B = 38;   // Surface
    constexpr BYTE SH_R = 59,  SH_G = 57,  SH_B = 64;   // Surface hover
    constexpr BYTE BD_R = 86,  BD_G = 83,  BD_B = 89;   // Border

    // Text
    constexpr BYTE TX_R = 242, TX_G = 238, TX_B = 244;  // Primary text
    constexpr BYTE TS_R = 153, TS_G = 150, TS_B = 156;  // Secondary text

    // Pink accent
    constexpr BYTE AC_R = 230, AC_G = 74,  AC_B = 129;  // Accent
    constexpr BYTE AH_R = 247, AH_G = 101, AH_B = 159;  // Accent hover
    constexpr BYTE AD_R = 102, AD_G = 46,  AD_B = 70;   // Accent dim

    // Danger (close)
    constexpr BYTE DN_R = 232, DN_G = 85,  DN_B = 85;

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
constexpr UINT WM_SHOW_TOOLBAR   = WM_USER + 21;

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
constexpr int TOOLBAR_GRIP_WIDTH  = 4;  // visible grip bar
constexpr int TOOLBAR_GRIP_AREA   = 12; // clickable strip housing the grip (padding + bar + padding)
constexpr int TOOLBAR_WIDTH       = TOOLBAR_NUM_BUTTONS * (TOOLBAR_BUTTON_SIZE + TOOLBAR_PADDING) + TOOLBAR_PADDING + TOOLBAR_GRIP_AREA;
