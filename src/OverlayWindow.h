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
#include <functional>

// Base class for full-screen overlay tools (Zoom, Draw, Snip, Timer)
class OverlayWindow {
public:
    virtual ~OverlayWindow();

    void Activate(HINSTANCE hInstance);
    void Dismiss();
    bool IsActive() const { return m_hwnd != nullptr; }
    void SetDismissCallback(std::function<void()> cb) { m_dismissCallback = std::move(cb); }

protected:
    // Subclasses override these
    virtual const wchar_t* ClassName() const = 0;
    virtual void OnPaint(HDC hdc, int width, int height) = 0;
    virtual void OnKeyDown(WPARAM /*vkey*/) {}
    virtual void OnMouseMove(int /*x*/, int /*y*/) {}
    virtual void OnMouseWheel(int /*delta*/, bool /*ctrlHeld*/) {}
    virtual void OnLButtonDown(int /*x*/, int /*y*/) {}
    virtual void OnLButtonUp(int /*x*/, int /*y*/) {}
    virtual void OnRButtonDown(int /*x*/, int /*y*/) {}
    virtual void OnTimer(UINT_PTR /*id*/) {}
    virtual void OnActivated() {}
    virtual void OnDismissed() {}

    void Invalidate();
    HWND GetHwnd() const { return m_hwnd; }

    // Screen capture helper
    static HBITMAP CaptureScreen(int& outX, int& outY, int& outW, int& outH);

    // Virtual screen bounds (set during Activate)
    int m_screenX = 0, m_screenY = 0, m_screenW = 0, m_screenH = 0;

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void RegisterWindowClass(HINSTANCE hInstance);

    HWND m_hwnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    bool m_classRegistered = false;
    std::function<void()> m_dismissCallback;
};
