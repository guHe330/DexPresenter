#pragma once
#include <Windows.h>
#include <shellapi.h>
#include <string>
#include "Constants.h"

class TrayIcon {
public:
    TrayIcon(HWND hwnd, HICON icon, const std::wstring& tooltip);
    ~TrayIcon();

    bool Show();
    void Hide();
    void UpdateTooltip(const std::wstring& tooltip);
    bool IsVisible() const { return m_visible; }

private:
    HWND hwnd;
    NOTIFYICONDATAW nid;
    bool m_visible = false;
};
