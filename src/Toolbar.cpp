#include "Toolbar.h"
#include "App.h"
#include "Constants.h"
#include <commctrl.h>
#include <windowsx.h>
#include <gdiplus.h>

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
    int x = cfg.posX;
    int y = cfg.posY;

    // Validate saved position is on a visible monitor
    bool needDefault = (x < 0);
    if (!needDefault) {
        RECT checkRect = { x, y, x + TOOLBAR_WIDTH, y + TOOLBAR_HEIGHT };
        HMONITOR hMon = MonitorFromRect(&checkRect, MONITOR_DEFAULTTONULL);
        needDefault = !hMon;
    }
    if (needDefault) {
        // Place above the taskbar in the lower-right corner of the primary work area
        RECT workArea;
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
        x = workArea.right - TOOLBAR_WIDTH - 8;
        y = workArea.bottom - TOOLBAR_HEIGHT - 8;
    }

    m_hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED,
        TOOLBAR_CLASS, L"DexPresenter",
        WS_POPUP,
        x, y, TOOLBAR_WIDTH, TOOLBAR_HEIGHT,
        nullptr, nullptr, hInstance, this);

    SetLayeredWindowAttributes(m_hwnd, 0, 230, LWA_ALPHA);

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

    if (cfg.visible) Show();
}

Toolbar::~Toolbar() {
    SavePosition();
    if (m_hwndTooltip) DestroyWindow(m_hwndTooltip);
    if (m_hwnd) DestroyWindow(m_hwnd);
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
    RECT rc;
    GetWindowRect(m_hwnd, &rc);
    m_app->GetConfig().toolbar.posX = rc.left;
    m_app->GetConfig().toolbar.posY = rc.top;
    m_app->GetConfig().toolbar.visible = IsVisible();
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
    auto& cfg = m_app->GetConfig();
    auto fmtHotkey = [](const HotkeyConfig& hk) -> std::wstring {
        std::wstring s;
        if (hk.modifiers & MOD_CONTROL) s += L"Ctrl+";
        if (hk.modifiers & MOD_ALT)     s += L"Alt+";
        if (hk.modifiers & MOD_SHIFT)   s += L"Shift+";
        if (hk.modifiers & MOD_WIN)     s += L"Win+";
        if (hk.vkey >= '0' && hk.vkey <= '9')
            s += (wchar_t)hk.vkey;
        else if (hk.vkey >= 'A' && hk.vkey <= 'Z')
            s += (wchar_t)hk.vkey;
        else {
            wchar_t buf[16];
            swprintf_s(buf, L"0x%02X", hk.vkey);
            s += buf;
        }
        return s;
    };
    switch (index) {
    case 0: return L"Snip (" + fmtHotkey(cfg.snip.hotkey) + L")";
    case 1: return L"Draw (" + fmtHotkey(cfg.draw.hotkey) + L")";
    case 2: return L"Timer (" + fmtHotkey(cfg.timer.hotkey) + L")";
    case 3: return L"Zoom (" + fmtHotkey(cfg.zoom.hotkey) + L")";
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

    // Button labels
    const wchar_t* labels[] = { L"S", L"D", L"T", L"Z", L"\x00D7" };

    Gdiplus::Font font(L"Segoe UI", 14.0f, Gdiplus::FontStyleBold);
    Gdiplus::StringFormat fmt;
    fmt.SetAlignment(Gdiplus::StringAlignmentCenter);
    fmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);

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

        // Text color: close=danger, others=accent when hovered, text when not
        Gdiplus::Color textColor;
        if (i == 4) {
            textColor = (i == m_hoverIndex) ? Theme::Danger() : Gdiplus::Color(255, Theme::DN_R, Theme::DN_G, Theme::DN_B);
        } else if (i == m_hoverIndex) {
            textColor = Theme::Accent();
        } else {
            textColor = Theme::Text();
        }
        Gdiplus::SolidBrush textBrush(textColor);

        graphics.DrawString(labels[i], -1, &font, btnRect, &fmt, &textBrush);
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
        // Clear to black (transparent via layered window)
        FillRect(hdcMem, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
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

    case WM_EXITSIZEMOVE:
        self->SavePosition();
        self->m_app->SaveConfig();
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
