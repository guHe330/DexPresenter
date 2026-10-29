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
    // Sync the Windows autorun (HKCU\...\Run) entry with m_config.startWithWindows.
    // The app is the sole owner of this registry value; the installer does not touch it.
    void ApplyAutostart();
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
    void ShowAbout();

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
