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
    virtual void OnKeyDown(WPARAM vkey) {}
    virtual void OnMouseMove(int x, int y) {}
    virtual void OnMouseWheel(int delta, bool ctrlHeld) {}
    virtual void OnLButtonDown(int x, int y) {}
    virtual void OnLButtonUp(int x, int y) {}
    virtual void OnRButtonDown(int x, int y) {}
    virtual void OnTimer(UINT_PTR id) {}
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
