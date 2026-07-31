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

#include "TimerTool.h"
#include "Config.h"
#include "Constants.h"

void TimerTool::OnActivated() {
    m_totalSeconds = m_config.durationSeconds;
    m_remainingSeconds = m_totalSeconds;
    m_finished = false;
    m_finishElapsedMs = 0;
    m_hintElapsedMs = 0;
    m_hintVisible = true;
    SetTimer(GetHwnd(), TIMER_ID, 1000, nullptr);
    SetTimer(GetHwnd(), HINT_TIMER_ID, HINT_TICK_MS, nullptr);
}

void TimerTool::OnDismissed() {
    if (GetHwnd()) {
        KillTimer(GetHwnd(), TIMER_ID);
        KillTimer(GetHwnd(), HINT_TIMER_ID);
    }
}

void TimerTool::OnPaint(HDC hdc, int width, int height) {
    // Determine background color based on finish state
    int bgR = 0, bgG = 0, bgB = 0;  // Default: black

    if (m_finished) {
        // Flash pattern: 2 cycles of (red + white), then constant white
        int totalFlashMs = FLASH_COUNT * 2 * FLASH_DURATION_MS;  // 2000ms for 2 flashes
        int cycleTime = m_finishElapsedMs % totalFlashMs;

        // Each cycle: 500ms red, 500ms white
        if (cycleTime < FLASH_DURATION_MS) {
            bgR = 255; bgG = 0; bgB = 0;      // Red
        } else if (cycleTime < FLASH_DURATION_MS * 2) {
            bgR = 255; bgG = 255; bgB = 255;  // White
        } else if (cycleTime < FLASH_DURATION_MS * 3) {
            bgR = 255; bgG = 0; bgB = 0;      // Red (2nd flash)
        } else {
            bgR = 255; bgG = 255; bgB = 255;  // White (then stays white)
        }
    }

    RECT rc = { 0, 0, width, height };
    HBRUSH hBrush = CreateSolidBrush(RGB(bgR, bgG, bgB));
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    // Large countdown text
    std::wstring text = FormatTime(m_remainingSeconds);

    int fontSize = height / 4; // Large font relative to screen
    HFONT hFont = CreateFontW(
        -fontSize, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, text.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Progress bar at bottom (hidden when finished)
    if (!m_finished) {
        float progress = (m_totalSeconds > 0)
            ? (float)(m_totalSeconds - m_remainingSeconds) / (float)m_totalSeconds
            : 0.0f;
        int barHeight = 6;
        int barWidth = (int)(width * progress);
        RECT barRect = { 0, height - barHeight, barWidth, height };
        HBRUSH hBarBrush = CreateSolidBrush(RGB(0, 120, 215));
        FillRect(hdc, &barRect, hBarBrush);
        DeleteObject(hBarBrush);
    }

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);

    // Hint overlay (visible for 4s after activation, fades out in the last second)
    if (m_hintVisible) {
        int alpha = 255;
        if (m_hintElapsedMs >= HINT_VISIBLE_MS) {
            int fadeProgress = m_hintElapsedMs - HINT_VISIBLE_MS;
            alpha = 255 - (int)(255.0f * fadeProgress / HINT_FADE_MS);
            if (alpha < 0) alpha = 0;
        }

        const wchar_t* hintText = L"Scroll: \u00B11 min  \u2502  Click or any key to close";
        int hintFontSize = height / 40 > 12 ? height / 40 : 12;
        HFONT hHintFont = CreateFontW(
            -hintFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        // Measure text size
        HDC hdcTmp = CreateCompatibleDC(hdc);
        HFONT hOldTmp = (HFONT)SelectObject(hdcTmp, hHintFont);
        SIZE textSize = {};
        GetTextExtentPoint32W(hdcTmp, hintText, (int)wcslen(hintText), &textSize);
        SelectObject(hdcTmp, hOldTmp);
        DeleteDC(hdcTmp);

        int pad = hintFontSize / 2;
        int boxW = textSize.cx + pad * 4;
        int boxH = textSize.cy + pad * 2;
        int boxX = (width - boxW) / 2;
        int boxY = height - boxH - height / 20;

        // Draw hint into an off-screen bitmap for alpha blending
        HDC hdcHint = CreateCompatibleDC(hdc);
        HBITMAP hHintBmp = CreateCompatibleBitmap(hdc, boxW, boxH);
        HBITMAP hOldHintBmp = (HBITMAP)SelectObject(hdcHint, hHintBmp);

        RECT boxRect = { 0, 0, boxW, boxH };
        HBRUSH hBoxBrush = CreateSolidBrush(RGB(30, 30, 30));
        FillRect(hdcHint, &boxRect, hBoxBrush);
        DeleteObject(hBoxBrush);

        HFONT hOldHintFont = (HFONT)SelectObject(hdcHint, hHintFont);
        SetTextColor(hdcHint, RGB(220, 220, 220));
        SetBkMode(hdcHint, TRANSPARENT);
        DrawTextW(hdcHint, hintText, -1, &boxRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdcHint, hOldHintFont);

        BLENDFUNCTION bf = {};
        bf.BlendOp = AC_SRC_OVER;
        bf.SourceConstantAlpha = (BYTE)alpha;
        bf.AlphaFormat = 0;
        AlphaBlend(hdc, boxX, boxY, boxW, boxH, hdcHint, 0, 0, boxW, boxH, bf);

        SelectObject(hdcHint, hOldHintBmp);
        DeleteObject(hHintBmp);
        DeleteDC(hdcHint);
        DeleteObject(hHintFont);
    }
}

void TimerTool::OnTimer(UINT_PTR id) {
    if (id == HINT_TIMER_ID) {
        m_hintElapsedMs += HINT_TICK_MS;
        if (m_hintElapsedMs >= HINT_VISIBLE_MS + HINT_FADE_MS) {
            m_hintVisible = false;
            KillTimer(GetHwnd(), HINT_TIMER_ID);
        }
        Invalidate();
        return;
    }

    if (id != TIMER_ID) return;

    if (!m_finished) {
        m_remainingSeconds--;
        if (m_remainingSeconds <= 0) {
            m_remainingSeconds = 0;
            m_finished = true;
            m_finishElapsedMs = 0;
        }
    } else {
        // Timer finished: track elapsed time for flashing
        m_finishElapsedMs += 1000;  // Timer fires every 1000ms

        // Total flash duration: 2 flashes * 2 phases (red+white) * 500ms = 2000ms
        // Then continue flashing indefinitely
    }
    Invalidate();
}

void TimerTool::OnKeyDown(WPARAM /*vkey*/) {
    // Any key dismisses (ESC handled by base class)
    Dismiss();
}

void TimerTool::OnLButtonDown(int /*x*/, int /*y*/) {
    Dismiss();
}

void TimerTool::OnMouseWheel(int delta, bool /*ctrlHeld*/) {
    // Adjust timer by ±1 minute per scroll, always snapping to whole minutes
    int minutes = m_remainingSeconds / 60;
    minutes += (delta > 0) ? 1 : -1;

    // Clamp to valid range (minimum 1 minute, maximum 24 hours)
    if (minutes < 1) minutes = 1;
    if (minutes > 1440) minutes = 1440;  // 1440 minutes = 24 hours

    m_remainingSeconds = minutes * 60;

    // Reset finish state so timer can continue
    if (m_finished) {
        m_finished = false;
        m_finishElapsedMs = 0;
    }

    Invalidate();
}
