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

// Menu command IDs
constexpr UINT IDM_SHOW_TOOLBAR = 201;
constexpr UINT IDM_SETTINGS     = 202;
constexpr UINT IDM_EXIT         = 203;

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

    CreateMessageWindow();

    // Create tray icon
    HICON hIcon = LoadIconW(m_hInstance, MAKEINTRESOURCEW(100)); // IDI_DEXPRESENTER
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
    RegisterHotKey(m_hWndMessage, HOTKEY_ZOOM,
        m_config.zoom.hotkey.modifiers, m_config.zoom.hotkey.vkey);
    RegisterHotKey(m_hWndMessage, HOTKEY_DRAW,
        m_config.draw.hotkey.modifiers, m_config.draw.hotkey.vkey);
    RegisterHotKey(m_hWndMessage, HOTKEY_TIMER,
        m_config.timer.hotkey.modifiers, m_config.timer.hotkey.vkey);
    RegisterHotKey(m_hWndMessage, HOTKEY_SNIP,
        m_config.snip.hotkey.modifiers, m_config.snip.hotkey.vkey);
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
    m_snipTool->Activate(m_hInstance);
}

void App::SaveConfig() {
    if (m_toolbar) m_toolbar->SavePosition();
    Config::Save(m_config);
}

void App::ShowSettings(int toolIndex) {
    // Map tool index: 0=zoom->tab1, 1=draw->tab2, 2=timer->tab3, 3=snip->tab4
    int tab = toolIndex + 1; // +1 because tab 0 is General
    if (tab < 0 || tab > 4) tab = 0;
    // Unregister hotkeys so key combos don't trigger tools while editing shortcuts
    UnregisterHotkeys();
    SettingsDialog::Show(this, m_hInstance, tab);
    RegisterHotkeys();
}

void App::ShowTrayMenu() {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, IDM_SHOW_TOOLBAR, L"Show Toolbar");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS, L"Settings...");
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
    case IDM_EXIT:
        PostQuitMessage(0);
        break;
    }
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
