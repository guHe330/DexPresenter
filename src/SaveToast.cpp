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

#include "SaveToast.h"
#include <string>

static constexpr int TOAST_VISIBLE_MS = 3000;
static constexpr int TOAST_FADE_MS = 700;
static constexpr int TOAST_TICK_MS = 50;
static constexpr UINT_PTR TOAST_TIMER = 1;

struct ToastData
{
    std::wstring line1; // "Image saved: filename.png"
    std::wstring line2; // directory path
    int elapsedMs = 0;
};

static LRESULT CALLBACK ToastWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ToastData *d = reinterpret_cast<ToastData *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg)
    {
    case WM_CREATE:
    {
        auto cs = reinterpret_cast<CREATESTRUCTW *>(lParam);
        d = reinterpret_cast<ToastData *>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(d));
        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
        SetTimer(hwnd, TOAST_TIMER, TOAST_TICK_MS, nullptr);
        return 0;
    }

    case WM_TIMER:
    {
        if (!d)
            return 0;
        d->elapsedMs += TOAST_TICK_MS;
        if (d->elapsedMs >= TOAST_VISIBLE_MS + TOAST_FADE_MS)
        {
            DestroyWindow(hwnd);
            return 0;
        }
        if (d->elapsedMs > TOAST_VISIBLE_MS)
        {
            int fade = d->elapsedMs - TOAST_VISIBLE_MS;
            int alpha = 255 - (255 * fade / TOAST_FADE_MS);
            if (alpha < 0)
                alpha = 0;
            SetLayeredWindowAttributes(hwnd, 0, (BYTE)alpha, LWA_ALPHA);
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }

    case WM_PAINT:
    {
        if (!d)
            break;
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        int W = rc.right, H = rc.bottom;

        // Use off-screen DC for clean rendering
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hBmp = CreateCompatibleBitmap(hdc, W, H);
        HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBmp);

        // Background — same dark pill as the hints
        HBRUSH hBg = CreateSolidBrush(RGB(30, 30, 30));
        FillRect(hdcMem, &rc, hBg);
        DeleteObject(hBg);

        SetBkMode(hdcMem, TRANSPARENT);

        int pad = H / 5;
        int mid = H / 2;
        int fontSize1 = H * 28 / 100; // ~28% of box height for main line
        int fontSize2 = H * 20 / 100; // ~20% for directory line

        // Line 1: "Image saved: filename.png"  (bright)
        HFONT hF1 = CreateFontW(-fontSize1, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        SelectObject(hdcMem, hF1);
        SetTextColor(hdcMem, RGB(235, 235, 235));
        RECT r1 = {pad, pad, W - pad, mid};
        DrawTextW(hdcMem, d->line1.c_str(), -1, &r1,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        DeleteObject(hF1);

        // Line 2: directory (grey, smaller)
        HFONT hF2 = CreateFontW(-fontSize2, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        SelectObject(hdcMem, hF2);
        SetTextColor(hdcMem, RGB(130, 130, 130));
        RECT r2 = {pad, mid, W - pad, H - pad};
        DrawTextW(hdcMem, d->line2.c_str(), -1, &r2,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        DeleteObject(hF2);

        BitBlt(hdc, 0, 0, W, H, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, hOld);
        DeleteObject(hBmp);
        DeleteDC(hdcMem);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        if (d)
        {
            KillTimer(hwnd, TOAST_TIMER);
            delete d;
        }
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void ShowSaveToast(const std::wstring &path)
{
    static bool registered = false;
    if (!registered)
    {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = ToastWndProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"DexPresenterToast";
        wc.style = CS_HREDRAW | CS_VREDRAW;
        RegisterClassExW(&wc);
        registered = true;
    }

    // Split path into directory and filename
    std::wstring dir, filename;
    size_t slash = path.rfind(L'\\');
    if (slash != std::wstring::npos)
    {
        dir = path.substr(0, slash);
        filename = path.substr(slash + 1);
    }
    else
    {
        dir = L".";
        filename = path;
    }

    // Abbreviate home directory to ~
    wchar_t profile[MAX_PATH] = {};
    DWORD profileLen = GetEnvironmentVariableW(L"USERPROFILE", profile, MAX_PATH);
    if (profileLen > 0 && dir.size() >= profileLen)
    {
        if (_wcsnicmp(dir.c_str(), profile, profileLen) == 0)
            dir = L"~" + dir.substr(profileLen);
    }

    // Size: same proportions as the hint boxes (screen-relative)
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // Font reference: hints use height/40 on a full overlay; here we pick a fixed
    // comfortable size that matches — roughly 1/16 of screen height for the box.
    int boxH = screenH / 16;
    int boxW = screenW / 2;
    int x = (screenW - boxW) / 2;
    int y = screenH - boxH - screenH / 20;

    ToastData *data = new ToastData();
    data->line1 = L"Image saved: " + filename;
    data->line2 = dir;

    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        L"DexPresenterToast", L"",
        WS_POPUP,
        x, y, boxW, boxH,
        nullptr, nullptr, GetModuleHandleW(nullptr), data);

    if (hwnd)
    {
        ShowWindow(hwnd, SW_SHOWNOACTIVATE);
        UpdateWindow(hwnd);
    }
    else
    {
        delete data;
    }
}
