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
    // Single-instance check
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, APP_MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Find existing instance and bring toolbar to front
        HWND existing = FindWindowW(APP_WINDOW_CLASS, nullptr);
        if (existing) {
            PostMessage(existing, WM_ACTIVATE_ZOOM, 0, 0);
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
