#include "ZoomTool.h"
#include "Constants.h"
#include <algorithm>

void ZoomTool::OnActivated() {
    int x, y, w, h;
    m_hScreen = CaptureScreen(x, y, w, h);
    m_zoomLevel = m_config.defaultZoomLevel;
    GetCursorPos(&m_panCenter);
    // Convert from screen coords to bitmap coords
    m_panCenter.x -= m_screenX;
    m_panCenter.y -= m_screenY;
}

void ZoomTool::OnDismissed() {
    if (m_hScreen) {
        DeleteObject(m_hScreen);
        m_hScreen = nullptr;
    }
}

void ZoomTool::OnPaint(HDC hdc, int width, int height) {
    if (!m_hScreen) return;

    Gdiplus::Graphics graphics(hdc);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    Gdiplus::Bitmap bitmap(m_hScreen, nullptr);

    float srcW = width / m_zoomLevel;
    float srcH = height / m_zoomLevel;
    float srcX = m_panCenter.x - srcW / 2.0f;
    float srcY = m_panCenter.y - srcH / 2.0f;

    // Clamp to screen bounds
    if (srcX < 0) srcX = 0;
    if (srcY < 0) srcY = 0;
    if (srcX + srcW > m_screenW) srcX = m_screenW - srcW;
    if (srcY + srcH > m_screenH) srcY = m_screenH - srcH;

    graphics.DrawImage(&bitmap,
        Gdiplus::RectF(0, 0, (float)width, (float)height),
        srcX, srcY, srcW, srcH,
        Gdiplus::UnitPixel);

    // Draw crosshair at center
    Gdiplus::Pen crossPen(Gdiplus::Color(128, 255, 255, 255), 1.0f);
    float cx = width / 2.0f;
    float cy = height / 2.0f;
    graphics.DrawLine(&crossPen, cx - 20, cy, cx + 20, cy);
    graphics.DrawLine(&crossPen, cx, cy - 20, cx, cy + 20);
}

void ZoomTool::OnMouseMove(int x, int y) {
    m_panCenter.x = x;
    m_panCenter.y = y;
    Invalidate();
}

void ZoomTool::OnMouseWheel(int delta, bool /*ctrlHeld*/) {
    float step = ZOOM_SCROLL_STEP * (delta > 0 ? 1.0f : -1.0f);
    m_zoomLevel = std::clamp(m_zoomLevel + step, ZOOM_MIN_LEVEL, ZOOM_MAX_LEVEL);
    Invalidate();
}
