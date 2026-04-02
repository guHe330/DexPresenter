#include "TimerTool.h"
#include "Config.h"
#include "Constants.h"

void TimerTool::OnActivated() {
    m_totalSeconds = m_config.durationSeconds;
    m_remainingSeconds = m_totalSeconds;
    m_finished = false;
    m_finishElapsedMs = 0;
    SetTimer(GetHwnd(), TIMER_ID, 1000, nullptr);
}

void TimerTool::OnDismissed() {
    if (GetHwnd()) KillTimer(GetHwnd(), TIMER_ID);
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
}

void TimerTool::OnTimer(UINT_PTR id) {
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

void TimerTool::OnKeyDown(WPARAM vkey) {
    // Any key dismisses (ESC handled by base class)
    Dismiss();
}

void TimerTool::OnLButtonDown(int x, int y) {
    Dismiss();
}

void TimerTool::OnMouseWheel(int delta, bool ctrlHeld) {
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
