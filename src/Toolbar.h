/**
 * DexPresenter - a free and open source Windows presentation utility with drawing tools
 * Copyright (C) 2026 Gunter Heiss
 *
 * For more information see: https://dexpresenter.com
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

    App* m_app;
    HINSTANCE m_hInstance;
    HWND m_hwnd = nullptr;
    HWND m_hwndTooltip = nullptr;
    int m_hoverIndex = -1;
    bool m_classRegistered = false;

    // Toolbar PNG icons loaded from embedded resources (indices match button order 0-3)
    Gdiplus::Bitmap* m_icons[4] = {};

    static constexpr int NUM_BUTTONS = 5; // Snip, Draw, Timer, Zoom, Close
    static constexpr int ICON_RENDER_W = 28;
    static constexpr int ICON_RENDER_H = 26;
};
