/**
 * DexPresenter - a free and open source Windows presentation utility with drawing tools
 * Copyright (C) 2026 Gunter Heiss
 *
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

#include <Windows.h>
#include <objbase.h>
#include <gdiplus.h>
#include <commctrl.h>
#include "App.h"
#include "Constants.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    // Opt into per-monitor DPI awareness before any window exists. Without it,
    // Windows virtualizes our coordinates on scaled displays (125%+ scaling,
    // typical on laptops): GetSystemMetrics returns scaled-down screen sizes,
    // so the Draw/Zoom/Snip captures cover only part of the real screen and
    // the overlays appear zoomed in with strokes offset from the cursor.
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Single-instance check
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, APP_MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Find existing instance and bring toolbar to front
        HWND existing = FindWindowW(APP_WINDOW_CLASS, nullptr);
        if (existing) {
            PostMessage(existing, WM_SHOW_TOOLBAR, 0, 0);
        }
        CloseHandle(hMutex);
        return 0;
    }

    // COM initialization
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // GDI+ initialization
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Common controls (tooltips, hotkey control, property sheets)
    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_BAR_CLASSES | ICC_STANDARD_CLASSES | ICC_HOTKEY_CLASS };
    InitCommonControlsEx(&icex);

    // Run application
    App app(hInstance);
    int result = app.Run();

    // Cleanup
    Gdiplus::GdiplusShutdown(gdiplusToken);
    CoUninitialize();
    CloseHandle(hMutex);
    return result;
}
