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

#include "Toolbar.h"
#include "App.h"
#include "Constants.h"
#include "../resources/resource.h"
#include <commctrl.h>
#include <windowsx.h>
#include <gdiplus.h>

static Gdiplus::Bitmap* LoadPngResource(HINSTANCE hInst, int resId) {
    HRSRC hRes = FindResourceW(hInst, MAKEINTRESOURCEW(resId), RT_RCDATA);
    if (!hRes) return nullptr;
    HGLOBAL hGlobal = LoadResource(hInst, hRes);
    if (!hGlobal) return nullptr;
    DWORD size = SizeofResource(hInst, hRes);
    void* data = LockResource(hGlobal);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hMem) return nullptr;
    void* mem = GlobalLock(hMem);
    memcpy(mem, data, size);
    GlobalUnlock(hMem);
    IStream* stream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(hMem, TRUE, &stream))) {
        GlobalFree(hMem);
        return nullptr;
    }
    Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromStream(stream);
    stream->Release();
    return bmp;
}

// Move a rect so it sits fully inside `bounds`, keeping its size.
static void ClampToBounds(RECT& rc, const RECT& bounds) {
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    int x = rc.left;
    int y = rc.top;
    // Push the far edge in first, then the near edge, so the top-left stays visible
    // even in the degenerate case of a monitor smaller than the toolbar
    if (x + w > bounds.right)  x = bounds.right - w;
    if (x < bounds.left)       x = bounds.left;
    if (y + h > bounds.bottom) y = bounds.bottom - h;
    if (y < bounds.top)        y = bounds.top;
    rc = { x, y, x + w, y + h };
}

// Identifies a monitor across a display change (e.g. "\\.\DISPLAY1"); empty on failure
static std::wstring MonitorDeviceName(HMONITOR hMon) {
    MONITORINFOEXW mi = {};
    mi.cbSize = sizeof(mi);
    if (!hMon || !GetMonitorInfoW(hMon, reinterpret_cast<MONITORINFO*>(&mi))) return {};
    return mi.szDevice;
}

static BOOL CALLBACK FindMonitorProc(HMONITOR hMon, HDC, LPRECT, LPARAM lParam) {
    auto* found = reinterpret_cast<std::pair<const std::wstring*, HMONITOR>*>(lParam);
    if (MonitorDeviceName(hMon) == *found->first) {
        found->second = hMon;
        return FALSE;
    }
    return TRUE;
}

// The monitor currently attached under this device name, or null if it is gone
static HMONITOR FindMonitorByName(const std::wstring& name) {
    if (name.empty()) return nullptr;
    std::pair<const std::wstring*, HMONITOR> found{ &name, nullptr };
    EnumDisplayMonitors(nullptr, nullptr, FindMonitorProc, reinterpret_cast<LPARAM>(&found));
    return found.second;
}

// Keep a window rect fully inside one monitor, optionally rubber-banding it to the
// work-area edges on the way. When useAnchor is set the monitor under `anchor` (the
// dragging cursor) wins, so crossing a boundary with the mouse hands the toolbar to
// the neighbouring screen instead of pinning it to the edge.
//
// snapDistance > 0 makes the work-area edges sticky: anywhere within that many pixels
// the rect sits flush against the edge, so parking the toolbar exactly on top of the
// taskbar takes no aim. Pull further than snapDistance and it releases, free to float
// over the taskbar out to the physical screen edge.
static void ConstrainRect(RECT& rc, POINT anchor, bool useAnchor, int snapDistance) {
    HMONITOR hMon = useAnchor
        ? MonitorFromPoint(anchor, MONITOR_DEFAULTTONEAREST)
        : MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    if (!GetMonitorInfoW(hMon, &mi)) return;

    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    int x = rc.left;
    int y = rc.top;

    if (snapDistance > 0) {
        const RECT& k = mi.rcWork;
        // Nearest edge per axis only — a rect can't be flush left and right at once
        if (abs(x - k.left) <= snapDistance)              x = k.left;
        else if (abs((x + w) - k.right) <= snapDistance)  x = k.right - w;
        if (abs(y - k.top) <= snapDistance)               y = k.top;
        else if (abs((y + h) - k.bottom) <= snapDistance) y = k.bottom - h;
    }

    rc = { x, y, x + w, y + h };
    ClampToBounds(rc, mi.rcMonitor);
}

static const wchar_t* TOOLBAR_CLASS = L"DexPresenterToolbar";

Toolbar::Toolbar(App* app, HINSTANCE hInstance) : m_app(app), m_hInstance(hInstance) {
    if (!m_classRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = TOOLBAR_CLASS;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassExW(&wc);
        m_classRegistered = true;
    }

    auto& cfg = app->GetConfig().toolbar;

    // Back onto the monitor it was left on, at the spot chosen there — or onto the
    // primary monitor if that one is no longer attached. A fresh config names no
    // monitor and holds no placements, so it resolves to the primary default corner.
    POINT pos{};
    std::wstring device;
    if (!ResolvePlacement(Utf8ToWide(cfg.monitor), pos, device)) {
        // No monitor could be queried at all — last-resort primary work area
        RECT workArea;
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
        pos = { workArea.right - TOOLBAR_WIDTH - TOOLBAR_EDGE_MARGIN,
                workArea.bottom - TOOLBAR_HEIGHT - TOOLBAR_EDGE_MARGIN };
    }
    int x = pos.x;
    int y = pos.y;

    m_hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED,
        TOOLBAR_CLASS, L"DexPresenter",
        WS_POPUP,
        x, y, TOOLBAR_WIDTH, TOOLBAR_HEIGHT,
        nullptr, nullptr, hInstance, this);

    SetLayeredWindowAttributes(m_hwnd, RGB(255, 0, 255), 230, LWA_ALPHA | LWA_COLORKEY);

    // Create tooltip control
    m_hwndTooltip = CreateWindowExW(
        WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr,
        WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        m_hwnd, nullptr, hInstance, nullptr);

    // Add tooltip tools for each button
    for (int i = 0; i < NUM_BUTTONS; i++) {
        TOOLINFOW ti = {};
        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_SUBCLASS;
        ti.hwnd = m_hwnd;
        ti.uId = i;
        ti.rect.left = TOOLBAR_PADDING + i * (TOOLBAR_BUTTON_SIZE + TOOLBAR_PADDING);
        ti.rect.top = TOOLBAR_PADDING;
        ti.rect.right = ti.rect.left + TOOLBAR_BUTTON_SIZE;
        ti.rect.bottom = ti.rect.top + TOOLBAR_BUTTON_SIZE;
        ti.lpszText = LPSTR_TEXTCALLBACKW;
        SendMessageW(m_hwndTooltip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
    }

    // Load toolbar icons from embedded resources (order: Snip, Draw, Timer, Zoom, Close)
    static const int iconResIds[NUM_BUTTONS] = {
        IDB_TOOLBAR_SNIP, IDB_TOOLBAR_DRAW, IDB_TOOLBAR_TIMER, IDB_TOOLBAR_ZOOM, IDB_TOOLBAR_CLOSE
    };
    for (int i = 0; i < NUM_BUTTONS; i++)
        m_icons[i] = LoadPngResource(hInstance, iconResIds[i]);

    // Note which monitor we ended up on before anything can move us off it. Not
    // SavePosition() — that would overwrite cfg.visible, which is read just below.
    RecordPlacement();

    if (cfg.visible) Show();
}

Toolbar::~Toolbar() {
    SavePosition();
    if (m_hwndTooltip) DestroyWindow(m_hwndTooltip);
    if (m_hwnd) DestroyWindow(m_hwnd);
    for (auto* bmp : m_icons) delete bmp;
}

void Toolbar::Show() {
    ShowWindow(m_hwnd, SW_SHOWNA);
    SavePosition();
    m_app->SaveConfig();
}

void Toolbar::Hide() {
    ShowWindow(m_hwnd, SW_HIDE);
    SavePosition();
    m_app->SaveConfig();
}

bool Toolbar::IsVisible() const {
    return m_hwnd && IsWindowVisible(m_hwnd);
}

void Toolbar::SavePosition() {
    if (!m_hwnd) return;
    RecordPlacement();
    m_app->GetConfig().toolbar.visible = IsVisible();
}

// Remember this spot as the user's choice for the monitor the toolbar is on, so a
// display change or a restart can put it back exactly there rather than somewhere
// merely valid. Persisted with the rest of the config.
void Toolbar::RecordPlacement() {
    if (!m_hwnd) return;
    RECT rc;
    GetWindowRect(m_hwnd, &rc);
    HMONITOR hMon = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
    std::wstring device = MonitorDeviceName(hMon);
    if (device.empty()) return;

    MONITORINFO mi = { sizeof(mi) };
    if (!GetMonitorInfoW(hMon, &mi)) return;

    m_currentDevice = device;
    auto& cfg = m_app->GetConfig().toolbar;
    cfg.monitor = WideToUtf8(device);
    cfg.placements[cfg.monitor] = { rc.left - mi.rcMonitor.left, rc.top - mi.rcMonitor.top };
}

bool Toolbar::ResolvePlacement(const std::wstring& preferredDevice, POINT& outPos,
                                std::wstring& outDevice) const {
    HMONITOR hMon = FindMonitorByName(preferredDevice);
    if (!hMon) hMon = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

    MONITORINFO mi = { sizeof(mi) };
    if (!GetMonitorInfoW(hMon, &mi)) return false;

    std::wstring device = MonitorDeviceName(hMon);
    const auto& placements = m_app->GetConfig().toolbar.placements;
    auto it = placements.find(WideToUtf8(device));

    POINT pos;
    if (it != placements.end()) {
        pos = { mi.rcMonitor.left + it->second.x, mi.rcMonitor.top + it->second.y };
    } else {
        // Never used this monitor — the same lower-right corner as a fresh install
        pos = { mi.rcWork.right - TOOLBAR_WIDTH - TOOLBAR_EDGE_MARGIN,
                mi.rcWork.bottom - TOOLBAR_HEIGHT - TOOLBAR_EDGE_MARGIN };
    }

    // The monitor may have changed resolution since the offset was stored
    RECT rc = { pos.x, pos.y, pos.x + TOOLBAR_WIDTH, pos.y + TOOLBAR_HEIGHT };
    ClampToBounds(rc, mi.rcMonitor);

    outPos = { rc.left, rc.top };
    outDevice = device;
    return true;
}

// Monitors were added, removed, or rearranged. Stay put if the toolbar's monitor
// survived; otherwise fall back to the primary one — at the position the user chose
// there, if they ever placed it there.
void Toolbar::OnDisplayChange() {
    if (!m_hwnd) return;

    POINT pos{};
    std::wstring device;
    if (!ResolvePlacement(m_currentDevice, pos, device)) return;

    SetWindowPos(m_hwnd, HWND_TOPMOST, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    SavePosition();
    m_app->SaveConfig();
}

int Toolbar::HitTest(int x, int y) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        int bx = TOOLBAR_PADDING + i * (TOOLBAR_BUTTON_SIZE + TOOLBAR_PADDING);
        int by = TOOLBAR_PADDING;
        if (x >= bx && x < bx + TOOLBAR_BUTTON_SIZE &&
            y >= by && y < by + TOOLBAR_BUTTON_SIZE) {
            return i;
        }
    }
    return -1;
}

std::wstring Toolbar::GetButtonTooltip(int index) {
    switch (index) {
    case 0: return L"Snip";
    case 1: return L"Draw";
    case 2: return L"Timer";
    case 3: return L"Zoom";
    case 4: return L"Minimize to tray";
    default: return L"";
    }
}

void Toolbar::Paint(HDC hdc) {
    RECT rc;
    GetClientRect(m_hwnd, &rc);

    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    // Background: dark rounded rect with theme colors
    Gdiplus::SolidBrush bgBrush(Theme::Background());
    Gdiplus::GraphicsPath path;
    int r = 8;
    path.AddArc(0, 0, r * 2, r * 2, 180, 90);
    path.AddArc(rc.right - r * 2, 0, r * 2, r * 2, 270, 90);
    path.AddArc(rc.right - r * 2, rc.bottom - r * 2, r * 2, r * 2, 0, 90);
    path.AddArc(0, rc.bottom - r * 2, r * 2, r * 2, 90, 90);
    path.CloseFigure();
    graphics.FillPath(&bgBrush, &path);

    // Subtle border
    Gdiplus::Pen borderPen(Theme::Border(100), 1.0f);
    graphics.DrawPath(&borderPen, &path);

    Gdiplus::Font closeFont(L"Segoe UI", 14.0f, Gdiplus::FontStyleBold);
    Gdiplus::StringFormat fmt;
    fmt.SetAlignment(Gdiplus::StringAlignmentCenter);
    fmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    // Drag grip: vertical bar inset on the right side
    {
        float gripX = (float)(rc.right - TOOLBAR_GRIP_AREA / 2 - TOOLBAR_GRIP_WIDTH / 2);
        float gripY = (float)(TOOLBAR_PADDING + 4);
        float gripH = (float)(TOOLBAR_BUTTON_SIZE - 8);
        Gdiplus::SolidBrush gripBrush(Theme::Border());
        graphics.FillRectangle(&gripBrush, gripX, gripY, (float)TOOLBAR_GRIP_WIDTH, gripH);
    }

    for (int i = 0; i < NUM_BUTTONS; i++) {
        int bx = TOOLBAR_PADDING + i * (TOOLBAR_BUTTON_SIZE + TOOLBAR_PADDING);
        int by = TOOLBAR_PADDING;
        Gdiplus::RectF btnRect((float)bx, (float)by,
            (float)TOOLBAR_BUTTON_SIZE, (float)TOOLBAR_BUTTON_SIZE);

        // Hover highlight
        if (i == m_hoverIndex) {
            Gdiplus::SolidBrush hoverBrush(Theme::SurfaceHover());
            float hr = 4.0f;
            Gdiplus::GraphicsPath hoverPath;
            hoverPath.AddArc(btnRect.X, btnRect.Y, hr * 2, hr * 2, 180, 90);
            hoverPath.AddArc(btnRect.X + btnRect.Width - hr * 2, btnRect.Y, hr * 2, hr * 2, 270, 90);
            hoverPath.AddArc(btnRect.X + btnRect.Width - hr * 2, btnRect.Y + btnRect.Height - hr * 2, hr * 2, hr * 2, 0, 90);
            hoverPath.AddArc(btnRect.X, btnRect.Y + btnRect.Height - hr * 2, hr * 2, hr * 2, 90, 90);
            hoverPath.CloseFigure();
            graphics.FillPath(&hoverBrush, &hoverPath);
        }

        if (m_icons[i]) {
            // Draw PNG icon centered in button at ICON_RENDER_W x ICON_RENDER_H
            float ix = btnRect.X + (btnRect.Width  - ICON_RENDER_W) / 2.0f;
            float iy = btnRect.Y + (btnRect.Height - ICON_RENDER_H) / 2.0f;
            graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            graphics.DrawImage(m_icons[i],
                Gdiplus::RectF(ix, iy, (float)ICON_RENDER_W, (float)ICON_RENDER_H));
        } else if (i == NUM_BUTTONS - 1) {
            // Close icon unavailable — fall back to a × glyph
            Gdiplus::Color textColor = (i == m_hoverIndex)
                ? Theme::Danger()
                : Gdiplus::Color(255, Theme::DN_R, Theme::DN_G, Theme::DN_B);
            Gdiplus::SolidBrush textBrush(textColor);
            graphics.DrawString(L"\x00D7", -1, &closeFont, btnRect, &fmt, &textBrush);
        }
    }
}

LRESULT CALLBACK Toolbar::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Toolbar* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Toolbar*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<Toolbar*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hBuf = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBuf);
        // Clear to magenta — keyed out by LWA_COLORKEY for true transparency outside the rounded background
        HBRUSH hKeyBrush = CreateSolidBrush(RGB(255, 0, 255));
        FillRect(hdcMem, &rc, hKeyBrush);
        DeleteObject(hKeyBrush);
        self->Paint(hdcMem);
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, hOld);
        DeleteObject(hBuf);
        DeleteDC(hdcMem);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int idx = self->HitTest(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        if (idx < 0) {
            // Drag the toolbar
            ReleaseCapture();
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
        return 0;
    }

    case WM_LBUTTONUP: {
        int idx = self->HitTest(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        switch (idx) {
        case 0: self->m_app->ActivateSnip();  break;
        case 1: self->m_app->ActivateDraw();  break;
        case 2: self->m_app->ActivateTimer(); break;
        case 3: self->m_app->ActivateZoom();  break;
        case 4: self->Hide(); break;
        }
        return 0;
    }

    case WM_RBUTTONUP: {
        int idx = self->HitTest(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // Map toolbar button to tool index for settings: 0=snip(3), 1=draw(1), 2=timer(2), 3=zoom(0)
        static const int toolMap[] = { 3, 1, 2, 0 };
        if (idx >= 0 && idx < 4) {
            self->m_app->ShowSettings(toolMap[idx]);
        }
        return 0;
    }

    case WM_MOUSEMOVE: {
        int idx = self->HitTest(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        if (idx != self->m_hoverIndex) {
            self->m_hoverIndex = idx;
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        TRACKMOUSEEVENT tme = {};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd;
        TrackMouseEvent(&tme);
        return 0;
    }

    case WM_MOVING: {
        // Constrain the drag so the toolbar is never partly off-screen, and let it
        // rubber-band onto the taskbar edge on the way
        RECT* rc = reinterpret_cast<RECT*>(lParam);
        POINT cursor{};
        bool useCursor = GetCursorPos(&cursor) && PtInRect(rc, cursor);
        ConstrainRect(*rc, cursor, useCursor, TOOLBAR_SNAP_DIST);
        return TRUE;
    }

    case WM_EXITSIZEMOVE:
        self->SavePosition();
        self->m_app->SaveConfig();
        return 0;

    case WM_DISPLAYCHANGE:
        self->OnDisplayChange();
        return 0;

    case WM_MOUSELEAVE:
        if (self->m_hoverIndex >= 0) {
            self->m_hoverIndex = -1;
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_NOTIFY: {
        auto nmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (nmhdr->code == TTN_GETDISPINFOW) {
            auto di = reinterpret_cast<NMTTDISPINFOW*>(lParam);
            static std::wstring tipBuf;
            tipBuf = self->GetButtonTooltip((int)di->hdr.idFrom);
            di->lpszText = tipBuf.data();
        }
        return 0;
    }

    case WM_DESTROY:
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
