#pragma once
#include "OverlayWindow.h"
#include "Config.h"

class SnipTool : public OverlayWindow {
public:
    void SetConfig(const SnipConfig& cfg) { m_config = cfg; }

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
    bool m_selecting = false;
    POINT m_startPt = {};
    POINT m_currentPt = {};
    POINT m_cursorPt = {};
    bool m_hasCursor = false;
};
