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
