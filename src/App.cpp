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

#include "App.h"
#include "TrayIcon.h"
#include "Toolbar.h"
#include "ZoomTool.h"
#include "DrawTool.h"
#include "TimerTool.h"
#include "SnipTool.h"
#include "SettingsDialog.h"
#include "Constants.h"
#include "Version.h"
#include <shellapi.h>
#include <string>

// Autorun registry location. The app owns this value exclusively; the installer
// no longer writes it, so the Settings toggle and actual autorun can never disagree.
static constexpr wchar_t kRunKey[]   = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static constexpr wchar_t kRunValue[] = L"DexPresenter";

// Menu command IDs
constexpr UINT IDM_SHOW_TOOLBAR = 201;
constexpr UINT IDM_SETTINGS     = 202;
constexpr UINT IDM_EXIT         = 203;
constexpr UINT IDM_ABOUT        = 204;

// Store App pointer for static WndProc
static App* g_app = nullptr;

App::App(HINSTANCE hInstance) : m_hInstance(hInstance) {
    g_app = this;
}

App::~App() {
    Shutdown();
    g_app = nullptr;
}

int App::Run() {
    Initialize();
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

void App::Initialize() {
    m_config = Config::Load();

    // Reconcile the autorun registry entry with the loaded setting on every
    // startup, so it stays correct even if the exe was moved or the value was
    // altered externally.
    ApplyAutostart();

    CreateMessageWindow();

    // Create tray icon — use small icon size so Windows picks the 32px pencil slot
    HICON hIcon = (HICON)LoadImageW(m_hInstance, MAKEINTRESOURCEW(100), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    if (!hIcon) hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    m_trayIcon = std::make_unique<TrayIcon>(m_hWndMessage, hIcon, L"DexPresenter");

    // Create tools
    m_zoomTool = std::make_unique<ZoomTool>();
    m_drawTool = std::make_unique<DrawTool>();
    m_timerTool = std::make_unique<TimerTool>();
    m_snipTool = std::make_unique<SnipTool>();

    // Create toolbar
    m_toolbar = std::make_unique<Toolbar>(this, m_hInstance);

    RegisterHotkeys();
}

void App::Shutdown() {
    UnregisterHotkeys();
    m_toolbar.reset();
    m_trayIcon.reset();
    m_zoomTool.reset();
    m_drawTool.reset();
    m_timerTool.reset();
    m_snipTool.reset();
    SaveConfig();
}

void App::CreateMessageWindow() {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = MessageWndProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = APP_WINDOW_CLASS;
    RegisterClassExW(&wc);

    m_hWndMessage = CreateWindowExW(
        0, APP_WINDOW_CLASS, APP_NAME,
        WS_POPUP, 0, 0, 0, 0,
        HWND_MESSAGE, nullptr, m_hInstance, nullptr);
}

void App::RegisterHotkeys() {
    // Hotkeys disabled in this release; code retained for re-enable later.
#if 0
    RegisterHotKey(m_hWndMessage, HOTKEY_ZOOM,
        m_config.zoom.hotkey.modifiers, m_config.zoom.hotkey.vkey);
    RegisterHotKey(m_hWndMessage, HOTKEY_DRAW,
        m_config.draw.hotkey.modifiers, m_config.draw.hotkey.vkey);
    RegisterHotKey(m_hWndMessage, HOTKEY_TIMER,
        m_config.timer.hotkey.modifiers, m_config.timer.hotkey.vkey);
    RegisterHotKey(m_hWndMessage, HOTKEY_SNIP,
        m_config.snip.hotkey.modifiers, m_config.snip.hotkey.vkey);
#endif
}

void App::UnregisterHotkeys() {
    if (!m_hWndMessage) return;
    UnregisterHotKey(m_hWndMessage, HOTKEY_ZOOM);
    UnregisterHotKey(m_hWndMessage, HOTKEY_DRAW);
    UnregisterHotKey(m_hWndMessage, HOTKEY_TIMER);
    UnregisterHotKey(m_hWndMessage, HOTKEY_SNIP);
}

void App::ReregisterHotkeys() {
    UnregisterHotkeys();
    RegisterHotkeys();
}

void App::ActivateZoom() {
    if (m_zoomTool->IsActive()) return;
    m_zoomTool->SetConfig(m_config.zoom);
    m_zoomTool->Activate(m_hInstance);
}

void App::ActivateDraw() {
    if (m_drawTool->IsActive()) return;
    // Hide toolbar so it doesn't appear in the screenshot
    bool toolbarWasVisible = m_toolbar && m_toolbar->IsVisible();
    if (toolbarWasVisible) m_toolbar->Hide();
    m_drawTool->SetConfig(m_config.draw);
    m_drawTool->SetScreenshotDir(m_config.screenshotDir);
    m_drawTool->SetDismissCallback([this, toolbarWasVisible]() {
        // Read back persisted state after tool is actually dismissed
        m_config.draw = m_drawTool->GetConfig();
        SaveConfig();
        if (toolbarWasVisible && m_toolbar) m_toolbar->Show();
    });
    m_drawTool->Activate(m_hInstance);
}

void App::ActivateTimer() {
    if (m_timerTool->IsActive()) return;
    m_timerTool->SetConfig(m_config.timer);
    m_timerTool->Activate(m_hInstance);
}

void App::ActivateSnip() {
    if (m_snipTool->IsActive()) return;
    m_snipTool->SetConfig(m_config.snip);
    m_snipTool->SetScreenshotDir(m_config.screenshotDir);
    m_snipTool->Activate(m_hInstance);
}

void App::SaveConfig() {
    if (m_toolbar) m_toolbar->SavePosition();
    Config::Save(m_config);
}

void App::ApplyAutostart() {
    HKEY hKey;
    // The Run key always exists on Windows; open it for writing.
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
        return;

    if (m_config.startWithWindows) {
        wchar_t exePath[MAX_PATH];
        DWORD len = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            // Quote the path so paths containing spaces launch correctly.
            std::wstring value = L"\"";
            value += exePath;
            value += L"\"";
            RegSetValueExW(hKey, kRunValue, 0, REG_SZ,
                reinterpret_cast<const BYTE*>(value.c_str()),
                static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
        }
    } else {
        // ERROR_FILE_NOT_FOUND (value already absent) is fine to ignore.
        RegDeleteValueW(hKey, kRunValue);
    }

    RegCloseKey(hKey);
}

void App::ShowSettings(int toolIndex) {
    // Tabs: 0=General, 1=Zoom, 2=Timer. Draw/Snip tools fall through to General.
    int tab = 0;
    if (toolIndex == 0)      tab = 1; // zoom
    else if (toolIndex == 2) tab = 2; // timer
    SettingsDialog::Show(this, m_hInstance, tab);
}

void App::ShowTrayMenu() {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, IDM_SHOW_TOOLBAR, L"Show Toolbar");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS, L"Settings...");
    AppendMenuW(hMenu, MF_STRING, IDM_ABOUT, L"About DexPresenter...");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"Exit");

    SetForegroundWindow(m_hWndMessage);
    UINT cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY,
        pt.x, pt.y, 0, m_hWndMessage, nullptr);
    DestroyMenu(hMenu);

    switch (cmd) {
    case IDM_SHOW_TOOLBAR:
        if (m_toolbar) m_toolbar->Show();
        break;
    case IDM_SETTINGS:
        ShowSettings(-1);
        break;
    case IDM_ABOUT:
        ShowAbout();
        break;
    case IDM_EXIT:
        PostQuitMessage(0);
        break;
    }
}

void App::ShowAbout() {
    std::wstring text =
        L"DexPresenter\n"
        L"Version " DEXPRESENTER_VERSION L"\n\n"
        L"A lightweight screen utility for presentations: snip, draw, timer, and zoom.\n\n"
        L"This program is free software: you can redistribute it and/or\n"
        L"modify it under the terms of the GNU General Public License\n"
        L"as published by the Free Software Foundation, either version 3\n"
        L"of the License, or (at your option) any later version.\n\n"
        L"Source code and full license: https://github.com/guHe330/DexPresenter";

    MessageBoxW(m_hWndMessage, text.c_str(), L"About DexPresenter", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK App::MessageWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!g_app) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_HOTKEY:
        switch (wParam) {
        case HOTKEY_ZOOM:  g_app->ActivateZoom();  break;
        case HOTKEY_DRAW:  g_app->ActivateDraw();  break;
        case HOTKEY_TIMER: g_app->ActivateTimer(); break;
        case HOTKEY_SNIP:  g_app->ActivateSnip();  break;
        }
        return 0;

    case WM_SHOW_TOOLBAR:
        if (g_app->m_toolbar && !g_app->m_toolbar->IsVisible())
            g_app->m_toolbar->Show();
        return 0;

    case WM_TRAYICON:
        switch (LOWORD(lParam)) {
        case WM_RBUTTONUP:
            g_app->ShowTrayMenu();
            break;
        case WM_LBUTTONDBLCLK:
            if (g_app->m_toolbar) g_app->m_toolbar->Show();
            break;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
