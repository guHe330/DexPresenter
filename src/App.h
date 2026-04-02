#pragma once
#include <Windows.h>
#include <memory>
#include "Config.h"

class TrayIcon;
class Toolbar;
class ZoomTool;
class DrawTool;
class TimerTool;
class SnipTool;

class App {
public:
    App(HINSTANCE hInstance);
    ~App();

    int Run(); // Main message loop

    void ActivateZoom();
    void ActivateDraw();
    void ActivateTimer();
    void ActivateSnip();

    AppConfig& GetConfig() { return m_config; }
    void SaveConfig();
    void ReregisterHotkeys();
    void ShowSettings(int toolIndex = 0);

    HINSTANCE GetInstance() const { return m_hInstance; }

private:
    void Initialize();
    void Shutdown();
    void RegisterHotkeys();
    void UnregisterHotkeys();
    void CreateMessageWindow();
    void ShowTrayMenu();

    static LRESULT CALLBACK MessageWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HINSTANCE m_hInstance;
    HWND m_hWndMessage = nullptr;

    AppConfig m_config;
    std::unique_ptr<TrayIcon> m_trayIcon;
    std::unique_ptr<Toolbar> m_toolbar;
    std::unique_ptr<ZoomTool> m_zoomTool;
    std::unique_ptr<DrawTool> m_drawTool;
    std::unique_ptr<TimerTool> m_timerTool;
    std::unique_ptr<SnipTool> m_snipTool;
};
