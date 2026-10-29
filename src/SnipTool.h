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
#include "OverlayWindow.h"
#include "Config.h"

class SnipTool : public OverlayWindow {
public:
    void SetConfig(const SnipConfig& cfg) { m_config = cfg; }
    void SetScreenshotDir(const std::string& dir) { m_screenshotDir = dir; }

protected:
    const wchar_t* ClassName() const override { return L"DexPresenterSnip"; }
    void OnPaint(HDC hdc, int width, int height) override;
    void OnMouseMove(int x, int y) override;
    void OnLButtonDown(int x, int y) override;
    void OnLButtonUp(int x, int y) override;
    void OnActivated() override;
    void OnDismissed() override;

private:
    void CopyRegionToClipboard(RECT sel);
    void SaveRegionToFile(RECT sel);
    void DrawMagnifier(Gdiplus::Graphics& g, HDC hdcScreen, int cx, int cy, int width, int height);
    RECT NormalizeRect(POINT a, POINT b);

    HBITMAP m_hScreen = nullptr;
    SnipConfig m_config;
    std::string m_screenshotDir;
    bool m_selecting = false;
    POINT m_startPt = {};
    POINT m_currentPt = {};
    POINT m_cursorPt = {};
    bool m_hasCursor = false;
};
