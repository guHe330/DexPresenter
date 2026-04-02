#include "TrayIcon.h"

TrayIcon::TrayIcon(HWND hwnd, HICON icon, const std::wstring& tooltip) : hwnd(hwnd) {
    ZeroMemory(&nid, sizeof(NOTIFYICONDATAW));
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = icon;
    wcsncpy_s(nid.szTip, tooltip.c_str(), _TRUNCATE);
    Show();
}

TrayIcon::~TrayIcon() {
    Hide();
}

bool TrayIcon::Show() {
    if (m_visible) {
        Shell_NotifyIconW(NIM_MODIFY, &nid);
        return true;
    }
    if (Shell_NotifyIconW(NIM_ADD, &nid)) {
        m_visible = true;
        return true;
    }
    return false;
}

void TrayIcon::Hide() {
    if (m_visible) {
        Shell_NotifyIconW(NIM_DELETE, &nid);
        m_visible = false;
    }
}

void TrayIcon::UpdateTooltip(const std::wstring& tooltip) {
    wcsncpy_s(nid.szTip, tooltip.c_str(), _TRUNCATE);
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}
