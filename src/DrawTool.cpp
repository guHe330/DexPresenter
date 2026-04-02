#include "DrawTool.h"
#include "SaveUtils.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>

// Color palette
static const COLORREF g_palette[] = {
    RGB(255, 0, 0),       // Red
    RGB(0, 200, 0),       // Green
    RGB(0, 0, 255),       // Blue
    RGB(255, 255, 0),     // Yellow
    RGB(255, 165, 0),     // Orange
    RGB(232, 96, 140),    // Theme pink
    RGB(255, 255, 255),   // White
    RGB(0, 0, 0),         // Black
};
static constexpr int NUM_PALETTE_COLORS = 8;

// Width presets
static const int g_widthPresets[] = { 1, 3, 6, 10 };
static constexpr int NUM_WIDTH_PRESETS = 4;

// Shape mode labels (Smooth, Obfuscate, and Eraser added; Eraser has no shortcut key shown)
static const wchar_t* g_shapeLabels[] = { L"Freehand", L"Smooth", L"Arrow", L"Box", L"Circle", L"Obfuscate", L"Eraser" };
static const wchar_t* g_shapeKeys[]   = { L"F",        L"S",      L"A",    L"B",  L"C",     L"V",          L"X" };
static constexpr int NUM_DRAW_MODES = 7;  // total shape modes incl. obfuscate and eraser

// Menu structure:
// Items 0..6:   shape modes (Freehand, Smooth, Arrow, Box, Circle, Obfuscate, Eraser)
// separator
// colors        (2 rows x 4 cols of swatches)
// separator
// widths        (4 dot presets)
// separator
// Undo          (action item, index 300)
// Redo          (action item, index 302)
// Clear All     (action item, index 301)
//
// For hit testing we use a different scheme — see ContextMenuHitTest.

const ShapeStyle& DrawTool::GetCurrentStyle() const {
    switch (m_shapeMode) {
    case ShapeMode::Freehand: return m_config.freehand;
    case ShapeMode::Arrow:    return m_config.arrow;
    case ShapeMode::Box:      return m_config.box;
    case ShapeMode::Circle:   return m_config.circle;
    case ShapeMode::Smooth:   return m_config.smooth;
    case ShapeMode::Obfuscate: return m_config.freehand; // uses freehand style
    case ShapeMode::Eraser:   return m_config.freehand; // eraser doesn't draw
    }
    return m_config.freehand;
}

ShapeStyle& DrawTool::GetMutableStyle() {
    switch (m_shapeMode) {
    case ShapeMode::Freehand: return m_config.freehand;
    case ShapeMode::Arrow:    return m_config.arrow;
    case ShapeMode::Box:      return m_config.box;
    case ShapeMode::Circle:   return m_config.circle;
    case ShapeMode::Smooth:   return m_config.smooth;
    case ShapeMode::Obfuscate: return m_config.freehand;
    case ShapeMode::Eraser:   return m_config.freehand;
    }
    return m_config.freehand;
}

void DrawTool::SaveCurrentStyle() {
    if (m_shapeMode == ShapeMode::Eraser || m_shapeMode == ShapeMode::Obfuscate) return;
    auto& s = GetMutableStyle();
    s.colorR = GetRValue(m_penColor);
    s.colorG = GetGValue(m_penColor);
    s.colorB = GetBValue(m_penColor);
    s.width = m_penWidth;
}

void DrawTool::ApplyCurrentStyle() {
    const auto& s = GetCurrentStyle();
    m_penColor = RGB(s.colorR, s.colorG, s.colorB);
    m_penWidth = s.width;
    if (m_shapeMode == ShapeMode::Eraser || m_shapeMode == ShapeMode::Obfuscate)
        SetCursor(LoadCursor(nullptr, IDC_HAND));
    else
        SetCursor(LoadCursor(nullptr, IDC_CROSS));
}

void DrawTool::OnActivated() {
    int x, y, w, h;
    m_hScreen = CaptureScreen(x, y, w, h);

    // Restore last used tool from config
    int mode = m_config.lastMode;
    if (mode >= 0 && mode < NUM_DRAW_MODES)
        m_shapeMode = (ShapeMode)mode;
    else
        m_shapeMode = ShapeMode::Freehand;
    // Don't restore to Eraser or Obfuscate — start with a drawing tool
    if (m_shapeMode == ShapeMode::Eraser || m_shapeMode == ShapeMode::Obfuscate)
        m_shapeMode = ShapeMode::Freehand;
    // Load per-shape color and width
    ApplyCurrentStyle();

    m_elements.clear();
    m_undoStack.clear();
    m_redoStack.clear();
    m_drawing = false;
    m_menuVisible = false;
    SetCursor(LoadCursor(nullptr, IDC_CROSS));
}

void DrawTool::OnDismissed() {
    // Save current tool's style and last-used state for next session
    SaveCurrentStyle();
    m_config.lastMode = (int)m_shapeMode;
    m_config.lastColorR = GetRValue(m_penColor);
    m_config.lastColorG = GetGValue(m_penColor);
    m_config.lastColorB = GetBValue(m_penColor);
    m_config.lastWidth = m_penWidth;

    if (!m_elements.empty()) {
        SaveScreenshot();
    }
    if (m_hScreen) {
        DeleteObject(m_hScreen);
        m_hScreen = nullptr;
    }
    m_elements.clear();
}

void DrawTool::OnPaint(HDC hdc, int width, int height) {
    if (!m_hScreen) return;

    // Draw captured screen
    HDC hdcSrc = CreateCompatibleDC(hdc);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcSrc, m_hScreen);
    BitBlt(hdc, 0, 0, width, height, hdcSrc, 0, 0, SRCCOPY);
    SelectObject(hdcSrc, hOld);
    DeleteDC(hdcSrc);

    // Draw elements on top using GDI+
    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    DrawAllElements(graphics);

    // Context menu
    if (m_menuVisible) {
        PaintContextMenu(hdc);
    }
}

void DrawTool::DrawAllElements(Gdiplus::Graphics& graphics) {
    for (auto& elem : m_elements)
        DrawElement(graphics, elem);
    if (m_drawing)
        DrawElement(graphics, m_currentElement);
}

void DrawTool::DrawElement(Gdiplus::Graphics& graphics, const ::DrawElement& elem) {
    Gdiplus::Color color(255, GetRValue(elem.color), GetGValue(elem.color), GetBValue(elem.color));
    Gdiplus::Pen pen(color, (float)elem.width);
    pen.SetStartCap(Gdiplus::LineCapRound);
    pen.SetEndCap(Gdiplus::LineCapRound);
    pen.SetLineJoin(Gdiplus::LineJoinRound);

    switch (elem.type) {
    case ShapeMode::Freehand: {
        if (elem.points.size() < 2) return;
        std::vector<Gdiplus::Point> pts;
        pts.reserve(elem.points.size());
        for (auto& p : elem.points)
            pts.push_back(Gdiplus::Point(p.x, p.y));
        graphics.DrawLines(&pen, pts.data(), (int)pts.size());
        break;
    }
    case ShapeMode::Arrow:
        DrawArrowShape(graphics, elem.startPt, elem.endPt, elem.color, elem.width);
        break;
    case ShapeMode::Box: {
        int x = (std::min)(elem.startPt.x, elem.endPt.x);
        int y = (std::min)(elem.startPt.y, elem.endPt.y);
        int w = abs(elem.endPt.x - elem.startPt.x);
        int h = abs(elem.endPt.y - elem.startPt.y);
        if (w > 0 && h > 0)
            graphics.DrawRectangle(&pen, x, y, w, h);
        break;
    }
    case ShapeMode::Circle: {
        int x = (std::min)(elem.startPt.x, elem.endPt.x);
        int y = (std::min)(elem.startPt.y, elem.endPt.y);
        int w = abs(elem.endPt.x - elem.startPt.x);
        int h = abs(elem.endPt.y - elem.startPt.y);
        if (w > 0 && h > 0)
            graphics.DrawEllipse(&pen, x, y, w, h);
        break;
    }
    case ShapeMode::Smooth:
        DrawSmoothCurve(graphics, elem);
        break;
    case ShapeMode::Obfuscate: {
        int left = (std::min)(elem.startPt.x, elem.endPt.x);
        int top = (std::min)(elem.startPt.y, elem.endPt.y);
        int right = (std::max)(elem.startPt.x, elem.endPt.x);
        int bottom = (std::max)(elem.startPt.y, elem.endPt.y);
        if (right - left < 2 || bottom - top < 2) break;

        HDC hdcSrc = CreateCompatibleDC(nullptr);
        HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcSrc, m_hScreen);

        int bx0 = left / OBFUSCATE_PIXEL_SIZE;
        int bx1 = right / OBFUSCATE_PIXEL_SIZE;
        int by0 = top / OBFUSCATE_PIXEL_SIZE;
        int by1 = bottom / OBFUSCATE_PIXEL_SIZE;

        for (int bx = bx0; bx <= bx1; bx++) {
            for (int by = by0; by <= by1; by++) {
                int px0 = bx * OBFUSCATE_PIXEL_SIZE;
                int py0 = by * OBFUSCATE_PIXEL_SIZE;
                // Average all pixels in this block
                long sumR = 0, sumG = 0, sumB = 0;
                int count = 0;
                for (int py = py0; py < py0 + OBFUSCATE_PIXEL_SIZE; py++) {
                    for (int px = px0; px < px0 + OBFUSCATE_PIXEL_SIZE; px++) {
                        COLORREF c = GetPixel(hdcSrc, px, py);
                        sumR += GetRValue(c);
                        sumG += GetGValue(c);
                        sumB += GetBValue(c);
                        count++;
                    }
                }
                BYTE avgR = (BYTE)(sumR / count);
                BYTE avgG = (BYTE)(sumG / count);
                BYTE avgB = (BYTE)(sumB / count);
                Gdiplus::SolidBrush brush(Gdiplus::Color(255, avgR, avgG, avgB));
                graphics.FillRectangle(&brush, px0, py0,
                    OBFUSCATE_PIXEL_SIZE, OBFUSCATE_PIXEL_SIZE);
            }
        }

        SelectObject(hdcSrc, hOldBmp);
        DeleteDC(hdcSrc);
        break;
    }
    case ShapeMode::Eraser:
        break; // Eraser doesn't draw
    }
}

void DrawTool::DrawArrowShape(Gdiplus::Graphics& graphics, POINT start, POINT end, COLORREF color, int width) {
    float dx = (float)(end.x - start.x);
    float dy = (float)(end.y - start.y);
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 2.0f) return;

    Gdiplus::Color gColor(255, GetRValue(color), GetGValue(color), GetBValue(color));
    float ux = dx / len, uy = dy / len;  // unit direction
    float px = -uy, py = ux;             // perpendicular

    float headLen = (std::max)(14.0f, (float)width * 4.5f);
    float headWidth = headLen * 0.45f;

    // Shaft: stop at the arrowhead base so the round cap doesn't poke past the tip
    float shaftEndX = (float)end.x - ux * headLen;
    float shaftEndY = (float)end.y - uy * headLen;

    if (len > headLen) {
        Gdiplus::Pen pen(gColor, (float)width);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapFlat);
        graphics.DrawLine(&pen, (float)start.x, (float)start.y, shaftEndX, shaftEndY);
    }

    // Arrowhead: filled triangle with tip at the exact endpoint
    Gdiplus::PointF tip((float)end.x, (float)end.y);
    Gdiplus::PointF left(end.x - ux * headLen + px * headWidth,
                         end.y - uy * headLen + py * headWidth);
    Gdiplus::PointF right(end.x - ux * headLen - px * headWidth,
                          end.y - uy * headLen - py * headWidth);

    Gdiplus::PointF arrowPts[] = { tip, left, right };
    Gdiplus::SolidBrush brush(gColor);
    graphics.FillPolygon(&brush, arrowPts, 3);
}

// Ramer-Douglas-Peucker point simplification
static float PerpendicularDist(POINT p, POINT a, POINT b) {
    float dx = (float)(b.x - a.x), dy = (float)(b.y - a.y);
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.001f) {
        float ex = (float)(p.x - a.x), ey = (float)(p.y - a.y);
        return sqrtf(ex * ex + ey * ey);
    }
    return fabsf(dy * (float)(p.x - a.x) - dx * (float)(p.y - a.y)) / len;
}

static void RDPSimplify(const std::vector<POINT>& pts, int start, int end,
                         float epsilon, std::vector<bool>& keep) {
    if (end - start < 2) return;
    float maxDist = 0;
    int maxIdx = start;
    for (int i = start + 1; i < end; i++) {
        float d = PerpendicularDist(pts[i], pts[start], pts[end]);
        if (d > maxDist) { maxDist = d; maxIdx = i; }
    }
    if (maxDist > epsilon) {
        keep[maxIdx] = true;
        RDPSimplify(pts, start, maxIdx, epsilon, keep);
        RDPSimplify(pts, maxIdx, end, epsilon, keep);
    }
}

static std::vector<POINT> SimplifyPoints(const std::vector<POINT>& pts, float epsilon) {
    if (pts.size() < 3) return pts;
    std::vector<bool> keep(pts.size(), false);
    keep.front() = true;
    keep.back() = true;
    RDPSimplify(pts, 0, (int)pts.size() - 1, epsilon, keep);
    std::vector<POINT> result;
    for (size_t i = 0; i < pts.size(); i++)
        if (keep[i]) result.push_back(pts[i]);
    return result;
}

void DrawTool::DrawSmoothCurve(Gdiplus::Graphics& graphics, const ::DrawElement& elem) {
    if (elem.points.size() < 2) return;

    Gdiplus::Color color(255, GetRValue(elem.color), GetGValue(elem.color), GetBValue(elem.color));
    Gdiplus::Pen pen(color, (float)elem.width);
    pen.SetStartCap(Gdiplus::LineCapRound);
    pen.SetEndCap(Gdiplus::LineCapRound);
    pen.SetLineJoin(Gdiplus::LineJoinRound);

    // Points are already pre-smoothed via exponential smoothing during input.
    // Render with Catmull-Rom bezier for silky curves.
    const auto& pts = elem.points;
    int n = (int)pts.size();

    if (n < 4) {
        std::vector<Gdiplus::Point> gpts;
        for (auto& p : pts)
            gpts.push_back(Gdiplus::Point(p.x, p.y));
        if (gpts.size() >= 2)
            graphics.DrawLines(&pen, gpts.data(), (int)gpts.size());
        return;
    }

    Gdiplus::GraphicsPath path;

    for (int i = 0; i < n - 1; i++) {
        // Four Catmull-Rom control points: p0, p1, p2, p3
        POINT p0 = pts[(std::max)(i - 1, 0)];
        POINT p1 = pts[i];
        POINT p2 = pts[(std::min)(i + 1, n - 1)];
        POINT p3 = pts[(std::min)(i + 2, n - 1)];

        // Convert to cubic bezier control points
        float c1x = (float)p1.x + ((float)p2.x - (float)p0.x) / 6.0f;
        float c1y = (float)p1.y + ((float)p2.y - (float)p0.y) / 6.0f;
        float c2x = (float)p2.x - ((float)p3.x - (float)p1.x) / 6.0f;
        float c2y = (float)p2.y - ((float)p3.y - (float)p1.y) / 6.0f;

        path.AddBezier(
            Gdiplus::PointF((float)p1.x, (float)p1.y),
            Gdiplus::PointF(c1x, c1y),
            Gdiplus::PointF(c2x, c2y),
            Gdiplus::PointF((float)p2.x, (float)p2.y));
    }

    graphics.DrawPath(&pen, &path);
}

int DrawTool::EraserHitTest(int x, int y) const {
    // Walk backwards so the topmost (last drawn) element gets erased first
    constexpr int HIT_DIST = 8;
    for (int i = (int)m_elements.size() - 1; i >= 0; i--) {
        const auto& elem = m_elements[i];
        switch (elem.type) {
        case ShapeMode::Freehand:
        case ShapeMode::Smooth:
            for (auto& p : elem.points) {
                int dx = p.x - x, dy = p.y - y;
                if (dx * dx + dy * dy < (HIT_DIST + elem.width) * (HIT_DIST + elem.width))
                    return i;
            }
            break;
        case ShapeMode::Obfuscate: {
            int left = (std::min)(elem.startPt.x, elem.endPt.x);
            int top = (std::min)(elem.startPt.y, elem.endPt.y);
            int right = (std::max)(elem.startPt.x, elem.endPt.x);
            int bottom = (std::max)(elem.startPt.y, elem.endPt.y);
            if (x >= left - HIT_DIST && x <= right + HIT_DIST &&
                y >= top - HIT_DIST && y <= bottom + HIT_DIST)
                return i;
            break;
        }
        case ShapeMode::Arrow: {
            // Check proximity to the line from start to end
            float ax = (float)(elem.endPt.x - elem.startPt.x);
            float ay = (float)(elem.endPt.y - elem.startPt.y);
            float len = sqrtf(ax * ax + ay * ay);
            if (len < 1.0f) break;
            float t = ((x - elem.startPt.x) * ax + (y - elem.startPt.y) * ay) / (len * len);
            t = std::clamp(t, 0.0f, 1.0f);
            float cx = elem.startPt.x + t * ax;
            float cy = elem.startPt.y + t * ay;
            float dx = x - cx, dy = y - cy;
            if (dx * dx + dy * dy < (HIT_DIST + elem.width) * (HIT_DIST + elem.width))
                return i;
            break;
        }
        case ShapeMode::Box: {
            int left = (std::min)(elem.startPt.x, elem.endPt.x);
            int top = (std::min)(elem.startPt.y, elem.endPt.y);
            int right = (std::max)(elem.startPt.x, elem.endPt.x);
            int bottom = (std::max)(elem.startPt.y, elem.endPt.y);
            int margin = HIT_DIST + elem.width;
            // Near any edge?
            bool nearHoriz = (x >= left - margin && x <= right + margin) &&
                             (abs(y - top) < margin || abs(y - bottom) < margin);
            bool nearVert = (y >= top - margin && y <= bottom + margin) &&
                            (abs(x - left) < margin || abs(x - right) < margin);
            if (nearHoriz || nearVert) return i;
            break;
        }
        case ShapeMode::Circle: {
            int cx2 = (elem.startPt.x + elem.endPt.x) / 2;
            int cy2 = (elem.startPt.y + elem.endPt.y) / 2;
            float rx = (float)abs(elem.endPt.x - elem.startPt.x) / 2.0f;
            float ry = (float)abs(elem.endPt.y - elem.startPt.y) / 2.0f;
            if (rx < 1 || ry < 1) break;
            float nx = (x - cx2) / rx, ny = (y - cy2) / ry;
            float dist = sqrtf(nx * nx + ny * ny);
            float margin = (float)(HIT_DIST + elem.width) / (std::min)(rx, ry);
            if (fabsf(dist - 1.0f) < margin) return i;
            break;
        }
        default: break;
        }
    }
    return -1;
}

// ==========================================================================
// Context Menu (owner-drawn, themed)
// ==========================================================================

// Layout:
//   [Shape modes]     4 items, MENU_ITEM_HEIGHT each
//   [separator]       MENU_SEPARATOR_HEIGHT
//   [color swatches]  2 rows of 4 swatches each
//   [separator]       MENU_SEPARATOR_HEIGHT
//   [width presets]   single row of 4 dots

int DrawTool::GetMenuTotalHeight() const {
    return NUM_DRAW_MODES * MENU_ITEM_HEIGHT
        + MENU_SEPARATOR_HEIGHT
        + MENU_SWATCH_ROWS * (MENU_SWATCH_SIZE + MENU_SWATCH_GAP) - MENU_SWATCH_GAP + 2 * MENU_PADDING
        + MENU_SEPARATOR_HEIGHT
        + MENU_WIDTH_DOT_SIZE + 2 * MENU_PADDING
        + MENU_SEPARATOR_HEIGHT
        + 3 * MENU_ITEM_HEIGHT;  // Undo + Redo + Clear All
}

void DrawTool::ShowContextMenu(int x, int y) {
    m_menuX = x;
    m_menuY = y;
    m_menuHover = -1;
    m_menuVisible = true;

    // Clamp so menu stays on screen
    int menuH = GetMenuTotalHeight();
    if (m_menuY + menuH > m_screenH) m_menuY = m_screenH - menuH - 4;
    if (m_menuX + MENU_ITEM_WIDTH > m_screenW) m_menuX = m_screenW - MENU_ITEM_WIDTH - 4;
    if (m_menuX < 0) m_menuX = 4;
    if (m_menuY < 0) m_menuY = 4;

    Invalidate();
}

void DrawTool::CloseContextMenu() {
    if (m_menuVisible) {
        m_menuVisible = false;
        m_menuHover = -1;
        Invalidate();
    }
}

void DrawTool::PaintContextMenu(HDC hdc) {
    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    float mx = (float)m_menuX, my = (float)m_menuY;
    float mw = (float)MENU_ITEM_WIDTH;
    float mh = (float)GetMenuTotalHeight();

    // Background with rounded corners
    Gdiplus::GraphicsPath bgPath;
    float r = 8.0f;
    bgPath.AddArc(mx, my, r * 2, r * 2, 180, 90);
    bgPath.AddArc(mx + mw - r * 2, my, r * 2, r * 2, 270, 90);
    bgPath.AddArc(mx + mw - r * 2, my + mh - r * 2, r * 2, r * 2, 0, 90);
    bgPath.AddArc(mx, my + mh - r * 2, r * 2, r * 2, 90, 90);
    bgPath.CloseFigure();

    // Drop shadow
    Gdiplus::SolidBrush shadowBrush(Gdiplus::Color(60, 0, 0, 0));
    Gdiplus::GraphicsPath shadowPath;
    float so = 3.0f;
    shadowPath.AddArc(mx + so, my + so, r * 2, r * 2, 180, 90);
    shadowPath.AddArc(mx + mw - r * 2 + so, my + so, r * 2, r * 2, 270, 90);
    shadowPath.AddArc(mx + mw - r * 2 + so, my + mh - r * 2 + so, r * 2, r * 2, 0, 90);
    shadowPath.AddArc(mx + so, my + mh - r * 2 + so, r * 2, r * 2, 90, 90);
    shadowPath.CloseFigure();
    graphics.FillPath(&shadowBrush, &shadowPath);

    Gdiplus::SolidBrush bgBrush(Gdiplus::Color(240, Theme::BG_R, Theme::BG_G, Theme::BG_B));
    graphics.FillPath(&bgBrush, &bgPath);
    Gdiplus::Pen borderPen(Theme::Border(150), 1.0f);
    graphics.DrawPath(&borderPen, &bgPath);

    Gdiplus::Font font(L"Segoe UI", 10.0f);
    Gdiplus::Font fontBold(L"Segoe UI", 10.0f, Gdiplus::FontStyleBold);
    Gdiplus::Font fontSmall(L"Segoe UI", 8.0f);
    Gdiplus::StringFormat fmtLeft;
    fmtLeft.SetAlignment(Gdiplus::StringAlignmentNear);
    fmtLeft.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    Gdiplus::StringFormat fmtRight;
    fmtRight.SetAlignment(Gdiplus::StringAlignmentFar);
    fmtRight.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    float curY = my;

    // --- Shape mode items ---
    for (int i = 0; i < NUM_DRAW_MODES; i++) {
        Gdiplus::RectF itemRect(mx, curY, mw, (float)MENU_ITEM_HEIGHT);
        bool isCurrent = ((int)m_shapeMode == i);
        bool isHover = (m_menuHover == i);

        if (isHover) {
            Gdiplus::SolidBrush hoverBrush(Theme::SurfaceHover());
            if (i == 0) {
                Gdiplus::GraphicsPath hp;
                hp.AddArc(mx, curY, r * 2, r * 2, 180, 90);
                hp.AddArc(mx + mw - r * 2, curY, r * 2, r * 2, 270, 90);
                hp.AddLine(mx + mw, curY + MENU_ITEM_HEIGHT, mx, curY + MENU_ITEM_HEIGHT);
                hp.CloseFigure();
                graphics.FillPath(&hoverBrush, &hp);
            } else {
                graphics.FillRectangle(&hoverBrush, itemRect);
            }
        }

        float textX = mx + MENU_PADDING + 20.0f;
        if (isCurrent) {
            Gdiplus::SolidBrush accentBrush(Theme::Accent());
            Gdiplus::RectF checkRect(mx + MENU_PADDING, curY, 18.0f, (float)MENU_ITEM_HEIGHT);
            graphics.DrawString(L"\u2713", -1, &fontBold, checkRect, &fmtLeft, &accentBrush);
        }

        Gdiplus::SolidBrush textBrush(isCurrent ? Theme::Accent() : Theme::Text());
        Gdiplus::RectF labelRect(textX, curY, mw - textX + mx - MENU_PADDING, (float)MENU_ITEM_HEIGHT);
        graphics.DrawString(g_shapeLabels[i], -1, isCurrent ? &fontBold : &font, labelRect, &fmtLeft, &textBrush);

        Gdiplus::SolidBrush keyBrush(Theme::TextSecondary());
        Gdiplus::RectF keyRect(mx, curY, mw - MENU_PADDING, (float)MENU_ITEM_HEIGHT);
        graphics.DrawString(g_shapeKeys[i], -1, &fontSmall, keyRect, &fmtRight, &keyBrush);

        curY += MENU_ITEM_HEIGHT;
    }

    // --- Separator ---
    float sepY = curY + MENU_SEPARATOR_HEIGHT / 2.0f;
    Gdiplus::Pen sepPen(Theme::Border(100), 1.0f);
    graphics.DrawLine(&sepPen, mx + MENU_PADDING, sepY, mx + mw - MENU_PADDING, sepY);
    curY += MENU_SEPARATOR_HEIGHT;

    // --- Color swatches (2 rows x 4 cols) ---
    float totalSwatchRowW = MENU_SWATCH_COLS * (MENU_SWATCH_SIZE + MENU_SWATCH_GAP) - MENU_SWATCH_GAP;
    float swatchStartX = mx + (mw - totalSwatchRowW) / 2.0f;
    float swatchBaseY = curY + MENU_PADDING;

    for (int i = 0; i < NUM_PALETTE_COLORS; i++) {
        int col = i % MENU_SWATCH_COLS;
        int row = i / MENU_SWATCH_COLS;
        float sx = swatchStartX + col * (MENU_SWATCH_SIZE + MENU_SWATCH_GAP);
        float sy = swatchBaseY + row * (MENU_SWATCH_SIZE + MENU_SWATCH_GAP);
        COLORREF c = g_palette[i];
        Gdiplus::SolidBrush swatchBrush(Gdiplus::Color(255, GetRValue(c), GetGValue(c), GetBValue(c)));

        float sr = 4.0f;
        Gdiplus::GraphicsPath sp;
        sp.AddArc(sx, sy, sr * 2, sr * 2, 180, 90);
        sp.AddArc(sx + MENU_SWATCH_SIZE - sr * 2, sy, sr * 2, sr * 2, 270, 90);
        sp.AddArc(sx + MENU_SWATCH_SIZE - sr * 2, sy + MENU_SWATCH_SIZE - sr * 2, sr * 2, sr * 2, 0, 90);
        sp.AddArc(sx, sy + MENU_SWATCH_SIZE - sr * 2, sr * 2, sr * 2, 90, 90);
        sp.CloseFigure();
        graphics.FillPath(&swatchBrush, &sp);

        bool isHover = (m_menuHover == 100 + i);
        if (c == m_penColor || isHover) {
            Gdiplus::Pen selPen(c == m_penColor ? Theme::Accent() : Theme::Border(180), 2.0f);
            graphics.DrawPath(&selPen, &sp);
        }
    }
    float swatchBlockH = MENU_SWATCH_ROWS * (MENU_SWATCH_SIZE + MENU_SWATCH_GAP) - MENU_SWATCH_GAP + 2 * MENU_PADDING;
    curY += swatchBlockH;

    // --- Separator ---
    sepY = curY + MENU_SEPARATOR_HEIGHT / 2.0f;
    graphics.DrawLine(&sepPen, mx + MENU_PADDING, sepY, mx + mw - MENU_PADDING, sepY);
    curY += MENU_SEPARATOR_HEIGHT;

    // --- Width presets row ---
    float widthRowH = (float)(MENU_WIDTH_DOT_SIZE + 2 * MENU_PADDING);
    float totalWidthW = NUM_WIDTH_PRESETS * (MENU_WIDTH_DOT_SIZE + MENU_SWATCH_GAP) - MENU_SWATCH_GAP;
    float widthStartX = mx + (mw - totalWidthW) / 2.0f;
    float widthY = curY + MENU_PADDING;

    for (int i = 0; i < NUM_WIDTH_PRESETS; i++) {
        float wx = widthStartX + i * (MENU_WIDTH_DOT_SIZE + MENU_SWATCH_GAP);
        int w = g_widthPresets[i];
        bool selected = (m_penWidth == w);
        bool isHover = (m_menuHover == 200 + i);

        // Background circle for hover/selection
        if (selected || isHover) {
            Gdiplus::SolidBrush bg(selected ? Theme::SurfaceHover() : Gdiplus::Color(40, 255, 255, 255));
            float bgR = 3.0f;
            Gdiplus::GraphicsPath bp;
            bp.AddArc(wx, widthY, bgR * 2, bgR * 2, 180, 90);
            bp.AddArc(wx + MENU_WIDTH_DOT_SIZE - bgR * 2, widthY, bgR * 2, bgR * 2, 270, 90);
            bp.AddArc(wx + MENU_WIDTH_DOT_SIZE - bgR * 2, widthY + MENU_WIDTH_DOT_SIZE - bgR * 2, bgR * 2, bgR * 2, 0, 90);
            bp.AddArc(wx, widthY + MENU_WIDTH_DOT_SIZE - bgR * 2, bgR * 2, bgR * 2, 90, 90);
            bp.CloseFigure();
            graphics.FillPath(&bg, &bp);
            if (selected) {
                Gdiplus::Pen selPen(Theme::Accent(), 1.5f);
                graphics.DrawPath(&selPen, &bp);
            }
        }

        // Dot representing the width
        float dotR = (std::min)((float)w * 1.5f, (float)MENU_WIDTH_DOT_SIZE * 0.4f);
        float cx = wx + MENU_WIDTH_DOT_SIZE / 2.0f;
        float cy = widthY + MENU_WIDTH_DOT_SIZE / 2.0f;
        Gdiplus::SolidBrush dotBrush(Theme::Text());
        graphics.FillEllipse(&dotBrush, cx - dotR, cy - dotR, dotR * 2, dotR * 2);
    }
    curY += widthRowH;

    // --- Separator ---
    sepY = curY + MENU_SEPARATOR_HEIGHT / 2.0f;
    graphics.DrawLine(&sepPen, mx + MENU_PADDING, sepY, mx + mw - MENU_PADDING, sepY);
    curY += MENU_SEPARATOR_HEIGHT;

    // --- Undo, Redo & Clear All ---
    struct ActionItem { const wchar_t* icon; const wchar_t* label; const wchar_t* shortcut; int id; bool enabled; };
    ActionItem actions[] = {
        { L"\u21B6", L"Undo",      L"Ctrl+Z", 300, !m_undoStack.empty() },
        { L"\u21B7", L"Redo",      L"Ctrl+Y", 302, !m_redoStack.empty() },
        { nullptr,   L"Clear All", L"E",      301, !m_elements.empty() },
    };
    for (auto& act : actions) {
        bool isHover = (m_menuHover == act.id);
        Gdiplus::RectF itemRect(mx, curY, mw, (float)MENU_ITEM_HEIGHT);

        if (isHover && act.enabled) {
            Gdiplus::SolidBrush hoverBrush(Theme::SurfaceHover());
            // Bottom-round the last item
            if (act.id == 301) {
                Gdiplus::GraphicsPath hp;
                hp.AddLine(mx, curY, mx + mw, curY);
                hp.AddArc(mx + mw - r * 2, curY + MENU_ITEM_HEIGHT - r * 2, r * 2, r * 2, 0, 90);
                hp.AddArc(mx, curY + MENU_ITEM_HEIGHT - r * 2, r * 2, r * 2, 90, 90);
                hp.CloseFigure();
                graphics.FillPath(&hoverBrush, &hp);
            } else {
                graphics.FillRectangle(&hoverBrush, itemRect);
            }
        }

        // Draw bent arrow icon for undo/redo
        float textX = mx + MENU_PADDING + 20.0f;
        if (act.icon) {
            Gdiplus::Color iconColor = act.enabled ? Theme::Accent() : Theme::TextSecondary(100);
            Gdiplus::SolidBrush iconBrush(iconColor);
            Gdiplus::RectF iconRect(mx + MENU_PADDING, curY, 18.0f, (float)MENU_ITEM_HEIGHT);
            graphics.DrawString(act.icon, -1, &fontBold, iconRect, &fmtLeft, &iconBrush);
        }

        Gdiplus::Color textColor = act.enabled ? Theme::Text() : Theme::TextSecondary(100);
        Gdiplus::SolidBrush textBrush(textColor);
        Gdiplus::RectF labelRect(textX, curY, mw - textX + mx - MENU_PADDING, (float)MENU_ITEM_HEIGHT);
        graphics.DrawString(act.label, -1, &font, labelRect, &fmtLeft, &textBrush);

        Gdiplus::SolidBrush keyBrush(Theme::TextSecondary(act.enabled ? 255 : 100));
        Gdiplus::RectF keyRect(mx, curY, mw - MENU_PADDING, (float)MENU_ITEM_HEIGHT);
        graphics.DrawString(act.shortcut, -1, &fontSmall, keyRect, &fmtRight, &keyBrush);

        curY += MENU_ITEM_HEIGHT;
    }
}

int DrawTool::ContextMenuHitTest(int x, int y) {
    if (!m_menuVisible) return -1;

    float mx = (float)m_menuX, my = (float)m_menuY;
    float mw = (float)MENU_ITEM_WIDTH;

    // Check if within menu bounds horizontally
    if (x < m_menuX || x > m_menuX + MENU_ITEM_WIDTH) return -1;

    float curY = my;

    // Shape modes (indices 0..5)
    for (int i = 0; i < NUM_DRAW_MODES; i++) {
        if (y >= (int)curY && y < (int)(curY + MENU_ITEM_HEIGHT))
            return i;
        curY += MENU_ITEM_HEIGHT;
    }

    // Separator
    curY += MENU_SEPARATOR_HEIGHT;

    // Color swatches (indices 100..107) — 2 rows x 4 cols
    float swatchBlockH = MENU_SWATCH_ROWS * (MENU_SWATCH_SIZE + MENU_SWATCH_GAP) - MENU_SWATCH_GAP + 2 * MENU_PADDING;
    if (y >= (int)curY && y < (int)(curY + swatchBlockH)) {
        float totalSwatchRowW = MENU_SWATCH_COLS * (MENU_SWATCH_SIZE + MENU_SWATCH_GAP) - MENU_SWATCH_GAP;
        float swatchStartX = mx + (mw - totalSwatchRowW) / 2.0f;
        float swatchBaseY = curY + MENU_PADDING;

        for (int i = 0; i < NUM_PALETTE_COLORS; i++) {
            int col = i % MENU_SWATCH_COLS;
            int row = i / MENU_SWATCH_COLS;
            float sx = swatchStartX + col * (MENU_SWATCH_SIZE + MENU_SWATCH_GAP);
            float sy = swatchBaseY + row * (MENU_SWATCH_SIZE + MENU_SWATCH_GAP);
            if (x >= (int)sx && x < (int)(sx + MENU_SWATCH_SIZE) &&
                y >= (int)sy && y < (int)(sy + MENU_SWATCH_SIZE))
                return 100 + i;
        }
        return -1;
    }
    curY += swatchBlockH;

    // Separator
    curY += MENU_SEPARATOR_HEIGHT;

    // Width presets (indices 200..203)
    float widthRowH = (float)(MENU_WIDTH_DOT_SIZE + 2 * MENU_PADDING);
    if (y >= (int)curY && y < (int)(curY + widthRowH)) {
        float totalWidthW = NUM_WIDTH_PRESETS * (MENU_WIDTH_DOT_SIZE + MENU_SWATCH_GAP) - MENU_SWATCH_GAP;
        float widthStartX = mx + (mw - totalWidthW) / 2.0f;
        float widthY = curY + MENU_PADDING;

        for (int i = 0; i < NUM_WIDTH_PRESETS; i++) {
            float wx = widthStartX + i * (MENU_WIDTH_DOT_SIZE + MENU_SWATCH_GAP);
            if (x >= (int)wx && x < (int)(wx + MENU_WIDTH_DOT_SIZE) &&
                y >= (int)widthY && y < (int)(widthY + MENU_WIDTH_DOT_SIZE))
                return 200 + i;
        }
        return -1;
    }
    curY += widthRowH;

    // Separator
    curY += MENU_SEPARATOR_HEIGHT;

    // Undo (300), Redo (302), Clear All (301)
    static const int actionIds[] = { 300, 302, 301 };
    for (int i = 0; i < 3; i++) {
        if (y >= (int)curY && y < (int)(curY + MENU_ITEM_HEIGHT))
            return actionIds[i];
        curY += MENU_ITEM_HEIGHT;
    }

    return -1;
}

// ==========================================================================
// Input handling
// ==========================================================================

void DrawTool::OnKeyDown(WPARAM vkey) {
    switch (vkey) {
    case 'F': SaveCurrentStyle(); m_shapeMode = ShapeMode::Freehand; ApplyCurrentStyle(); CloseContextMenu(); Invalidate(); return;
    case 'A': SaveCurrentStyle(); m_shapeMode = ShapeMode::Arrow;    ApplyCurrentStyle(); CloseContextMenu(); Invalidate(); return;
    case 'B': SaveCurrentStyle(); m_shapeMode = ShapeMode::Box;      ApplyCurrentStyle(); CloseContextMenu(); Invalidate(); return;
    case 'C': SaveCurrentStyle(); m_shapeMode = ShapeMode::Circle;   ApplyCurrentStyle(); CloseContextMenu(); Invalidate(); return;
    case 'S': SaveCurrentStyle(); m_shapeMode = ShapeMode::Smooth;   ApplyCurrentStyle(); CloseContextMenu(); Invalidate(); return;
    case 'V': SaveCurrentStyle(); m_shapeMode = ShapeMode::Obfuscate; ApplyCurrentStyle(); CloseContextMenu(); Invalidate(); return;
    case 'X': SaveCurrentStyle(); m_shapeMode = ShapeMode::Eraser;   ApplyCurrentStyle(); CloseContextMenu(); Invalidate(); return;
    }

    // Ctrl+key combos first (before plain key shortcuts)
    bool ctrlHeld = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    if (ctrlHeld && vkey == 'Z') {
        if (!m_undoStack.empty()) {
            auto action = std::move(m_undoStack.back());
            m_undoStack.pop_back();
            if (action.type == UndoAction::Draw) {
                if (!m_elements.empty()) {
                    UndoAction redoAction;
                    redoAction.type = UndoAction::Draw;
                    redoAction.erasedElement = std::move(m_elements.back());
                    m_redoStack.push_back(std::move(redoAction));
                    m_elements.pop_back();
                }
            } else {
                int idx = std::clamp(action.erasedIndex, 0, (int)m_elements.size());
                UndoAction redoAction;
                redoAction.type = UndoAction::Erase;
                redoAction.erasedIndex = idx;
                m_redoStack.push_back(std::move(redoAction));
                m_elements.insert(m_elements.begin() + idx, std::move(action.erasedElement));
            }
            Invalidate();
        }
        return;
    }
    if (ctrlHeld && vkey == 'Y') {
        if (!m_redoStack.empty()) {
            auto action = std::move(m_redoStack.back());
            m_redoStack.pop_back();
            if (action.type == UndoAction::Draw) {
                m_elements.push_back(std::move(action.erasedElement));
                m_undoStack.push_back({ UndoAction::Draw });
            } else {
                int idx = std::clamp(action.erasedIndex, 0, (int)m_elements.size() - 1);
                UndoAction undoAction;
                undoAction.type = UndoAction::Erase;
                undoAction.erasedIndex = idx;
                undoAction.erasedElement = std::move(m_elements[idx]);
                m_undoStack.push_back(std::move(undoAction));
                m_elements.erase(m_elements.begin() + idx);
            }
            Invalidate();
        }
        return;
    }

    switch (vkey) {
    case 'R': m_penColor = RGB(255, 0, 0);     break;
    case 'G': m_penColor = RGB(0, 200, 0);     break;
    case 'Y': m_penColor = RGB(255, 255, 0);   break;
    case 'O': m_penColor = RGB(255, 165, 0);   break;
    case 'P': m_penColor = RGB(232, 96, 140);  break;
    case 'W': m_penColor = RGB(255, 255, 255); break;
    case 'K': m_penColor = RGB(0, 0, 0);       break;
    case 'E':
        m_elements.clear();
        m_undoStack.clear();
        m_redoStack.clear();
        Invalidate();
        break;
    }
}

void DrawTool::OnMouseMove(int x, int y) {
    if (m_menuVisible) {
        int hit = ContextMenuHitTest(x, y);
        if (hit != m_menuHover) {
            m_menuHover = hit;
            Invalidate();
        }
        return;
    }

    // Eraser drag: remove elements as cursor passes over them
    if (m_drawing && m_shapeMode == ShapeMode::Eraser) {
        int idx = EraserHitTest(x, y);
        if (idx >= 0) {
            UndoAction action;
            action.type = UndoAction::Erase;
            action.erasedIndex = idx;
            action.erasedElement = std::move(m_elements[idx]);
            m_undoStack.push_back(std::move(action));
            m_redoStack.clear();
            m_elements.erase(m_elements.begin() + idx);
            Invalidate();
        }
        return;
    }

    if (m_drawing) {
        if (m_shapeMode == ShapeMode::Smooth) {
            // Exponential smoothing: blend raw input toward running average
            m_smoothX = SMOOTH_ALPHA * (float)x + (1.0f - SMOOTH_ALPHA) * m_smoothX;
            m_smoothY = SMOOTH_ALPHA * (float)y + (1.0f - SMOOTH_ALPHA) * m_smoothY;
            // Only add point if it moved enough (reduces density)
            if (!m_currentElement.points.empty()) {
                auto& last = m_currentElement.points.back();
                float dx = m_smoothX - (float)last.x;
                float dy = m_smoothY - (float)last.y;
                if (dx * dx + dy * dy >= SMOOTH_MIN_DIST * SMOOTH_MIN_DIST) {
                    m_currentElement.points.push_back({ (int)m_smoothX, (int)m_smoothY });
                }
            } else {
                m_currentElement.points.push_back({ (int)m_smoothX, (int)m_smoothY });
            }
        } else if (m_shapeMode == ShapeMode::Freehand) {
            m_currentElement.points.push_back({ x, y });
        } else {
            POINT end = { x, y };
            if (GetKeyState(VK_SHIFT) & 0x8000) {
                int dx = abs(end.x - m_currentElement.startPt.x);
                int dy = abs(end.y - m_currentElement.startPt.y);
                int side = (std::max)(dx, dy);
                end.x = m_currentElement.startPt.x + (end.x >= m_currentElement.startPt.x ? side : -side);
                end.y = m_currentElement.startPt.y + (end.y >= m_currentElement.startPt.y ? side : -side);
            }
            m_currentElement.endPt = end;
        }
        Invalidate();
    }
}

void DrawTool::OnMouseWheel(int delta, bool ctrlHeld) {
    // Mouse wheel disabled for DrawTool
}

void DrawTool::OnLButtonDown(int x, int y) {
    // Handle context menu clicks
    if (m_menuVisible) {
        int hit = ContextMenuHitTest(x, y);
        if (hit >= 0 && hit < NUM_DRAW_MODES) {
            SaveCurrentStyle();
            m_shapeMode = (ShapeMode)hit;
            ApplyCurrentStyle();
            Invalidate();
            return;
        } else if (hit >= 100 && hit < 100 + NUM_PALETTE_COLORS) {
            m_penColor = g_palette[hit - 100];
            SaveCurrentStyle();
            Invalidate();
            return;
        } else if (hit >= 200 && hit < 200 + NUM_WIDTH_PRESETS) {
            m_penWidth = g_widthPresets[hit - 200];
            SaveCurrentStyle();
            Invalidate();
            return;
        } else if (hit == 300) { // Undo
            if (!m_undoStack.empty()) {
                auto action = std::move(m_undoStack.back());
                m_undoStack.pop_back();
                if (action.type == UndoAction::Draw) {
                    if (!m_elements.empty()) {
                        UndoAction redoAction;
                        redoAction.type = UndoAction::Draw;
                        redoAction.erasedElement = std::move(m_elements.back());
                        m_redoStack.push_back(std::move(redoAction));
                        m_elements.pop_back();
                    }
                } else {
                    int idx = std::clamp(action.erasedIndex, 0, (int)m_elements.size());
                    UndoAction redoAction;
                    redoAction.type = UndoAction::Erase;
                    redoAction.erasedIndex = idx;
                    m_redoStack.push_back(std::move(redoAction));
                    m_elements.insert(m_elements.begin() + idx, std::move(action.erasedElement));
                }
                Invalidate();
            }
            return;
        } else if (hit == 302) { // Redo
            if (!m_redoStack.empty()) {
                auto action = std::move(m_redoStack.back());
                m_redoStack.pop_back();
                if (action.type == UndoAction::Draw) {
                    m_elements.push_back(std::move(action.erasedElement));
                    m_undoStack.push_back({ UndoAction::Draw });
                } else {
                    int idx = std::clamp(action.erasedIndex, 0, (int)m_elements.size() - 1);
                    UndoAction undoAction;
                    undoAction.type = UndoAction::Erase;
                    undoAction.erasedIndex = idx;
                    undoAction.erasedElement = std::move(m_elements[idx]);
                    m_undoStack.push_back(std::move(undoAction));
                    m_elements.erase(m_elements.begin() + idx);
                }
                Invalidate();
            }
            return;
        } else if (hit == 301) { // Clear All
            m_elements.clear();
            m_undoStack.clear();
            m_redoStack.clear();
            Invalidate();
            return;
        }
        // Clicked outside menu — close it and fall through to start drawing
        CloseContextMenu();
    }

    // Eraser mode: drag to erase elements under the cursor
    if (m_shapeMode == ShapeMode::Eraser) {
        m_drawing = true;
        int idx = EraserHitTest(x, y);
        if (idx >= 0) {
            UndoAction action;
            action.type = UndoAction::Erase;
            action.erasedIndex = idx;
            action.erasedElement = std::move(m_elements[idx]);
            m_undoStack.push_back(std::move(action));
            m_redoStack.clear();
            m_elements.erase(m_elements.begin() + idx);
            Invalidate();
        }
        SetCapture(GetHwnd());
        return;
    }

    m_drawing = true;
    m_currentElement = {};
    m_currentElement.type = m_shapeMode;
    m_currentElement.color = m_penColor;
    m_currentElement.width = m_penWidth;
    m_currentElement.startPt = { x, y };
    m_currentElement.endPt = { x, y };
    if (m_shapeMode == ShapeMode::Freehand) {
        m_currentElement.points.push_back({ x, y });
    } else if (m_shapeMode == ShapeMode::Smooth) {
        m_smoothX = (float)x;
        m_smoothY = (float)y;
        m_currentElement.points.push_back({ x, y });
    }
    SetCapture(GetHwnd());
}

void DrawTool::OnLButtonUp(int x, int y) {
    if (m_drawing && m_shapeMode == ShapeMode::Eraser) {
        m_drawing = false;
        ReleaseCapture();
        return;
    }
    if (m_drawing) {
        if (m_shapeMode == ShapeMode::Smooth) {
            // Add final smoothed endpoint
            m_smoothX = SMOOTH_ALPHA * (float)x + (1.0f - SMOOTH_ALPHA) * m_smoothX;
            m_smoothY = SMOOTH_ALPHA * (float)y + (1.0f - SMOOTH_ALPHA) * m_smoothY;
            m_currentElement.points.push_back({ (int)m_smoothX, (int)m_smoothY });
        } else if (m_shapeMode == ShapeMode::Freehand) {
            m_currentElement.points.push_back({ x, y });
        } else {
            m_currentElement.endPt = { x, y };
        }
        m_elements.push_back(std::move(m_currentElement));
        m_undoStack.push_back({ UndoAction::Draw });
        m_redoStack.clear();
        m_currentElement = {};
        m_drawing = false;
        ReleaseCapture();
        Invalidate();
    }
}

void DrawTool::OnRButtonDown(int x, int y) {
    if (m_menuVisible) {
        CloseContextMenu();
    } else {
        ShowContextMenu(x, y);
    }
}

// ==========================================================================
// Screenshot compositing & saving
// ==========================================================================

HBITMAP DrawTool::CompositeFinalImage() {
    if (!m_hScreen) return nullptr;

    BITMAP bm;
    GetObject(m_hScreen, sizeof(bm), &bm);
    int w = bm.bmWidth, h = bm.bmHeight;

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hResult = CreateCompatibleBitmap(hdcScreen, w, h);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hResult);

    HDC hdcSrc = CreateCompatibleDC(hdcScreen);
    HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, m_hScreen);
    BitBlt(hdcMem, 0, 0, w, h, hdcSrc, 0, 0, SRCCOPY);
    SelectObject(hdcSrc, hOldSrc);
    DeleteDC(hdcSrc);

    {
        Gdiplus::Graphics graphics(hdcMem);
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        for (auto& elem : m_elements)
            DrawElement(graphics, elem);
    }

    SelectObject(hdcMem, hOld);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    return hResult;
}

void DrawTool::SaveScreenshot() {
    HBITMAP hComposite = CompositeFinalImage();
    if (!hComposite) return;

    std::wstring dir;
    if (!m_config.screenshotDir.empty()) {
        dir = Utf8ToWide(m_config.screenshotDir);
    } else {
        dir = GetDefaultScreenshotDir();
    }

    std::wstring path = GenerateScreenshotPath(dir, L"DexPresenter_Draw");
    SaveBitmapAsPng(hComposite, path);
    DeleteObject(hComposite);
}
