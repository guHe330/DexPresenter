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
#include "OverlayWindow.h"
#include "Config.h"

class ZoomTool : public OverlayWindow {
public:
    void SetConfig(const ZoomConfig& cfg) { m_config = cfg; }
    const ZoomConfig& GetConfig() const { return m_config; }

protected:
    const wchar_t* ClassName() const override { return L"DexPresenterOverlay"; }
    void OnPaint(HDC hdc, int width, int height) override;
    void OnMouseMove(int x, int y) override;
    void OnMouseWheel(int delta, bool ctrlHeld) override;
    void OnActivated() override;
    void OnDismissed() override;

private:
    HBITMAP m_hScreen = nullptr;
    float m_zoomLevel = 2.0f;
    POINT m_panCenter = {};
    ZoomConfig m_config;
};
