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

#include "OverlayWindow.h"
#include "Constants.h"
#include <windowsx.h>

OverlayWindow::~OverlayWindow() {
    Dismiss();
}

void OverlayWindow::RegisterWindowClass(HINSTANCE hInstance) {
    if (m_classRegistered) return;
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = ClassName();
    wc.hCursor = LoadCursor(nullptr, IDC_CROSS);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClassExW(&wc);
    m_classRegistered = true;
}

void OverlayWindow::Activate(HINSTANCE hInstance) {
    if (m_hwnd) return; // Already active

    m_hInstance = hInstance;
    RegisterWindowClass(hInstance);

    m_screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    m_screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);
    m_screenW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    m_screenH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        ClassName(), L"",
        WS_POPUP,
        m_screenX, m_screenY, m_screenW, m_screenH,
        nullptr, nullptr, hInstance, this);

    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_SHOW);
        SetForegroundWindow(m_hwnd);
        SetFocus(m_hwnd);
        OnActivated();
    }
}

void OverlayWindow::Dismiss() {
    if (m_hwnd) {
        HWND hwnd = m_hwnd;
        m_hwnd = nullptr;
        DestroyWindow(hwnd);
        OnDismissed();
        if (m_dismissCallback) m_dismissCallback();
    }
}

void OverlayWindow::Invalidate() {
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

HBITMAP OverlayWindow::CaptureScreen(int& outX, int& outY, int& outW, int& outH) {
    outX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    outY = GetSystemMetrics(SM_YVIRTUALSCREEN);
    outW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    outH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, outW, outH);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBmp);
    BitBlt(hdcMem, 0, 0, outW, outH, hdcScreen, outX, outY, SRCCOPY);
    SelectObject(hdcMem, hOld);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
    return hBmp;
}

LRESULT CALLBACK OverlayWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    OverlayWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<OverlayWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<OverlayWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        // Double-buffer. The buffer is as large as the whole virtual screen and the
        // window never resizes, so it is kept for the window's lifetime instead of
        // being allocated and freed on every mouse move.
        if (!self->m_hdcBuf) {
            self->m_hdcBuf = CreateCompatibleDC(hdc);
            self->m_hBuf = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            self->m_hOldBuf = (HBITMAP)SelectObject(self->m_hdcBuf, self->m_hBuf);
        }
        self->OnPaint(self->m_hdcBuf, rc.right, rc.bottom);
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, self->m_hdcBuf, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            self->Dismiss();
            return 0;
        }
        self->OnKeyDown(wParam);
        return 0;

    case WM_MOUSEMOVE:
        self->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        bool ctrl = (GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != 0;
        self->OnMouseWheel(delta, ctrl);
        return 0;
    }

    case WM_LBUTTONDOWN:
        self->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_LBUTTONUP:
        self->OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_RBUTTONDOWN:
        self->OnRButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_TIMER:
        self->OnTimer(wParam);
        return 0;

    case WM_DESTROY:
        self->m_hwnd = nullptr;
        if (self->m_hdcBuf) {
            SelectObject(self->m_hdcBuf, self->m_hOldBuf);
            DeleteObject(self->m_hBuf);
            DeleteDC(self->m_hdcBuf);
            self->m_hdcBuf = nullptr;
            self->m_hBuf = nullptr;
        }
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
