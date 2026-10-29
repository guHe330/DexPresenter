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
#include <gdiplus.h>
#include <string>

class App;

class Toolbar {
public:
    Toolbar(App* app, HINSTANCE hInstance);
    ~Toolbar();

    void Show();
    void Hide();
    bool IsVisible() const;
    void SavePosition();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void Paint(HDC hdc);
    int HitTest(int x, int y);
    std::wstring GetButtonTooltip(int index);
    void RecordPlacement();
    void OnDisplayChange();
    // Where the toolbar belongs given the monitors attached right now: on
    // preferredDevice if that is still one of them, otherwise on the primary.
    // False if the monitor could not be queried at all.
    bool ResolvePlacement(const std::wstring& preferredDevice, POINT& outPos,
                          std::wstring& outDevice) const;

    App* m_app;
    HINSTANCE m_hInstance;
    HWND m_hwnd = nullptr;
    HWND m_hwndTooltip = nullptr;
    int m_hoverIndex = -1;
    bool m_classRegistered = false;

    // Display device name of the monitor the toolbar is on right now. The placements
    // themselves live in ToolbarConfig so they persist across restarts.
    std::wstring m_currentDevice;

    static constexpr int NUM_BUTTONS = 5; // Snip, Draw, Timer, Zoom, Close

    // Toolbar PNG icons loaded from embedded resources (indices match button order)
    Gdiplus::Bitmap* m_icons[NUM_BUTTONS] = {};

    // Icons are 32x32 and drawn 1:1 inside the 36px button, so they never resample
    static constexpr int ICON_RENDER_W = 32;
    static constexpr int ICON_RENDER_H = 32;
};
