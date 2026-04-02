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
    static constexpr int FLASH_DURATION_MS = 500;      // 500ms per flash (red or white)
    static constexpr int FLASH_COUNT = 2;              // 2 complete flashes (red+white pairs)
    static constexpr int FLASH_INTERVAL_MS = 1000;     // 1 second between flashes

    TimerConfig m_config;
    int m_totalSeconds = 600;
    int m_remainingSeconds = 600;

    bool m_finished = false;
    int m_finishElapsedMs = 0;  // Elapsed time since timer finished (for flashing logic)
};
