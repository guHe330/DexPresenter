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

class TimerTool : public OverlayWindow {
public:
    void SetConfig(const TimerConfig& cfg) { m_config = cfg; }

protected:
    const wchar_t* ClassName() const override { return L"DexPresenterTimer"; }
    void OnPaint(HDC hdc, int width, int height) override;
    void OnKeyDown(WPARAM vkey) override;
    void OnMouseWheel(int delta, bool ctrlHeld) override;
    void OnLButtonDown(int x, int y) override;
    void OnTimer(UINT_PTR id) override;
    void OnActivated() override;
    void OnDismissed() override;

private:
    static constexpr UINT_PTR TIMER_ID = 100;
    static constexpr UINT_PTR HINT_TIMER_ID = 101;
    static constexpr int FLASH_DURATION_MS = 500;      // 500ms per flash (red or white)
    static constexpr int FLASH_COUNT = 2;              // 2 complete flashes (red+white pairs)
    static constexpr int FLASH_INTERVAL_MS = 1000;     // 1 second between flashes
    static constexpr int HINT_VISIBLE_MS = 3000;       // Full opacity duration
    static constexpr int HINT_FADE_MS = 1000;          // Fade-out duration
    static constexpr int HINT_TICK_MS = 50;            // Fade timer interval

    TimerConfig m_config;
    int m_totalSeconds = 600;
    int m_remainingSeconds = 600;

    bool m_finished = false;
    int m_finishElapsedMs = 0;  // Elapsed time since timer finished (for flashing logic)

    int m_hintElapsedMs = 0;    // Time since activation (for hint fade)
    bool m_hintVisible = true;
};
