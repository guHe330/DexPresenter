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

#include "SnipTool.h"
#include "SaveUtils.h"
#include "SaveToast.h"
#include "Constants.h"
#include <algorithm>

void SnipTool::OnActivated() {
    int x, y, w, h;
    m_hScreen = CaptureScreen(x, y, w, h);
    m_selecting = false;
    SetCursor(LoadCursor(nullptr, IDC_CROSS));
}

void SnipTool::OnDismissed() {
    if (m_hScreen) {
        DeleteObject(m_hScreen);
        m_hScreen = nullptr;
    }
}

RECT SnipTool::NormalizeRect(POINT a, POINT b) {
    RECT r;
    r.left   = (std::min)(a.x, b.x);
    r.top    = (std::min)(a.y, b.y);
    r.right  = (std::max)(a.x, b.x);
    r.bottom = (std::max)(a.y, b.y);
    return r;
}

void SnipTool::OnPaint(HDC hdc, int width, int height) {
    if (!m_hScreen) return;

    // Draw captured screen
    HDC hdcSrc = CreateCompatibleDC(hdc);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcSrc, m_hScreen);
    BitBlt(hdc, 0, 0, width, height, hdcSrc, 0, 0, SRCCOPY);
    SelectObject(hdcSrc, hOld);
    DeleteDC(hdcSrc);

    // Dim the entire screen
    Gdiplus::Graphics graphics(hdc);
    Gdiplus::SolidBrush dimBrush(Gdiplus::Color(100, 0, 0, 0));
    graphics.FillRectangle(&dimBrush, 0, 0, width, height);

    // If selecting, draw the selected region at full brightness
    if (m_selecting) {
        RECT sel = NormalizeRect(m_startPt, m_currentPt);
        int sw = sel.right - sel.left;
        int sh = sel.bottom - sel.top;

        if (sw > 0 && sh > 0) {
            // Redraw the selection area from original (undimmed)
            HDC hdcSrc2 = CreateCompatibleDC(hdc);
            HBITMAP hOld2 = (HBITMAP)SelectObject(hdcSrc2, m_hScreen);
            BitBlt(hdc, sel.left, sel.top, sw, sh, hdcSrc2, sel.left, sel.top, SRCCOPY);
            SelectObject(hdcSrc2, hOld2);
            DeleteDC(hdcSrc2);

            // Draw selection border
            Gdiplus::Pen borderPen(Gdiplus::Color(255, 0, 120, 215), 2.0f);
            graphics.DrawRectangle(&borderPen, sel.left, sel.top, sw, sh);

            // Show dimensions text
            wchar_t dimText[64];
            swprintf_s(dimText, L"%d x %d", sw, sh);
            Gdiplus::Font font(L"Segoe UI", 10.0f);
            Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
            Gdiplus::SolidBrush textBg(Gdiplus::Color(180, 0, 0, 0));
            Gdiplus::RectF textRect((float)sel.left, (float)sel.top - 22.0f, 120.0f, 20.0f);
            graphics.FillRectangle(&textBg, textRect);
            graphics.DrawString(dimText, -1, &font, Gdiplus::PointF((float)sel.left + 4, (float)sel.top - 20.0f), &textBrush);
        }
    }

    // Draw full-screen crosshair lines at cursor
    if (m_hasCursor) {
        Gdiplus::Pen guidePen(Gdiplus::Color(180, 0, 120, 215), 1.0f);
        graphics.DrawLine(&guidePen, 0, m_cursorPt.y, width, m_cursorPt.y);
        graphics.DrawLine(&guidePen, m_cursorPt.x, 0, m_cursorPt.x, height);
    }

    // Draw magnifier loupe near cursor (after selection so it's on top)
    if (m_hasCursor && m_hScreen) {
        HDC hdcMagSrc = CreateCompatibleDC(hdc);
        HBITMAP hOldMagSrc = (HBITMAP)SelectObject(hdcMagSrc, m_hScreen);
        DrawMagnifier(graphics, hdcMagSrc, m_cursorPt.x, m_cursorPt.y, width, height);
        SelectObject(hdcMagSrc, hOldMagSrc);
        DeleteDC(hdcMagSrc);
    }
}

void SnipTool::OnMouseMove(int x, int y) {
    m_cursorPt = { x, y };
    m_hasCursor = true;
    if (m_selecting) {
        m_currentPt = { x, y };
    }
    Invalidate();
}

void SnipTool::OnLButtonDown(int x, int y) {
    m_selecting = true;
    m_startPt = { x, y };
    m_currentPt = { x, y };
    SetCapture(GetHwnd());
}

void SnipTool::OnLButtonUp(int x, int y) {
    if (m_selecting) {
        m_currentPt = { x, y };
        m_selecting = false;
        ReleaseCapture();

        RECT sel = NormalizeRect(m_startPt, m_currentPt);
        int sw = sel.right - sel.left;
        int sh = sel.bottom - sel.top;

        if (sw > 2 && sh > 2) {
            CopyRegionToClipboard(sel);
            SaveRegionToFile(sel);
        }
        Dismiss();
    }
}

void SnipTool::CopyRegionToClipboard(RECT sel) {
    int w = sel.right - sel.left;
    int h = sel.bottom - sel.top;

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcSrc = CreateCompatibleDC(hdcScreen);
    HDC hdcDst = CreateCompatibleDC(hdcScreen);
    HBITMAP hSnip = CreateCompatibleBitmap(hdcScreen, w, h);

    HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, m_hScreen);
    HBITMAP hOldDst = (HBITMAP)SelectObject(hdcDst, hSnip);
    BitBlt(hdcDst, 0, 0, w, h, hdcSrc, sel.left, sel.top, SRCCOPY);
    SelectObject(hdcSrc, hOldSrc);
    SelectObject(hdcDst, hOldDst);

    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, hSnip);
        CloseClipboard();
        // Clipboard owns hSnip now, don't delete it
    } else {
        DeleteObject(hSnip);
    }

    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
    ReleaseDC(nullptr, hdcScreen);
}

void SnipTool::SaveRegionToFile(RECT sel) {
    int w = sel.right - sel.left;
    int h = sel.bottom - sel.top;

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcSrc = CreateCompatibleDC(hdcScreen);
    HDC hdcDst = CreateCompatibleDC(hdcScreen);
    HBITMAP hSnip = CreateCompatibleBitmap(hdcScreen, w, h);

    HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, m_hScreen);
    HBITMAP hOldDst = (HBITMAP)SelectObject(hdcDst, hSnip);
    BitBlt(hdcDst, 0, 0, w, h, hdcSrc, sel.left, sel.top, SRCCOPY);
    SelectObject(hdcSrc, hOldSrc);
    SelectObject(hdcDst, hOldDst);
    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
    ReleaseDC(nullptr, hdcScreen);

    std::wstring dir;
    if (!m_screenshotDir.empty()) {
        dir = Utf8ToWide(m_screenshotDir);
    } else {
        dir = GetDefaultScreenshotDir();
    }

    std::wstring path = GenerateScreenshotPath(dir, L"DexPresenter_Snip");
    SaveBitmapAsPng(hSnip, path);
    DeleteObject(hSnip);
    ShowSaveToast(path);
}

void SnipTool::DrawMagnifier(Gdiplus::Graphics& g, HDC hdcScreen, int cx, int cy, int width, int height) {
    // Total panel footprint
    int panelW = MAG_SIZE + 6;
    int panelH = MAG_SIZE + 6;

    // Try all four quadrants around the cursor: BR, BL, TR, TL
    struct { int dx, dy; } offsets[] = {
        { MAG_OFFSET,                  MAG_OFFSET },                   // bottom-right
        { -(MAG_OFFSET + panelW),      MAG_OFFSET },                   // bottom-left
        { MAG_OFFSET,                  -(MAG_OFFSET + panelH) },       // top-right
        { -(MAG_OFFSET + panelW),      -(MAG_OFFSET + panelH) },       // top-left
    };

    int mx = 4, my = 4; // fallback
    for (auto& o : offsets) {
        int tx = cx + o.dx;
        int ty = cy + o.dy;
        if (tx >= 0 && ty >= 0 && tx + panelW <= width && ty + panelH <= height) {
            mx = tx;
            my = ty;
            break;
        }
    }

    // Source region from the original screenshot
    int srcX = cx - MAG_SRC_SIZE / 2;
    int srcY = cy - MAG_SRC_SIZE / 2;

    // Create magnified bitmap via StretchBlt
    HDC hdcMag = CreateCompatibleDC(hdcScreen);
    HBITMAP hMag = CreateCompatibleBitmap(hdcScreen, MAG_SIZE, MAG_SIZE);
    HBITMAP hOldMag = (HBITMAP)SelectObject(hdcMag, hMag);
    SetStretchBltMode(hdcMag, COLORONCOLOR);
    StretchBlt(hdcMag, 0, 0, MAG_SIZE, MAG_SIZE,
               hdcScreen, srcX, srcY, MAG_SRC_SIZE, MAG_SRC_SIZE, SRCCOPY);

    SelectObject(hdcMag, hOldMag);

    // -- Draw panel background (rounded rect, themed) --
    Gdiplus::GraphicsPath panelPath;
    Gdiplus::RectF panelRect((float)mx - 3, (float)my - 3, (float)MAG_SIZE + 6, (float)MAG_SIZE + 6);
    float r = (float)MAG_CORNER_RAD;
    panelPath.AddArc(panelRect.X, panelRect.Y, r * 2, r * 2, 180, 90);
    panelPath.AddArc(panelRect.X + panelRect.Width - r * 2, panelRect.Y, r * 2, r * 2, 270, 90);
    panelPath.AddArc(panelRect.X + panelRect.Width - r * 2, panelRect.Y + panelRect.Height - r * 2, r * 2, r * 2, 0, 90);
    panelPath.AddArc(panelRect.X, panelRect.Y + panelRect.Height - r * 2, r * 2, r * 2, 90, 90);
    panelPath.CloseFigure();

    Gdiplus::SolidBrush bgBrush(Theme::Background(230));
    g.FillPath(&bgBrush, &panelPath);
    Gdiplus::Pen borderPen(Theme::Accent(), 2.0f);
    g.DrawPath(&borderPen, &panelPath);

    // -- Draw the magnified image --
    Gdiplus::Bitmap magBmp(hMag, nullptr);
    g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    g.DrawImage(&magBmp, mx, my, MAG_SIZE, MAG_SIZE);
    g.SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeDefault);

    // -- Draw crosshair in the center of the magnified area --
    int crossCX = mx + MAG_SIZE / 2;
    int crossCY = my + MAG_SIZE / 2;
    int halfCell = MAG_ZOOM / 2;

    // Dark outline for visibility, then pink accent crosshair
    Gdiplus::Pen shadowPen(Gdiplus::Color(120, 0, 0, 0), 3.0f);
    Gdiplus::Pen crossPen(Theme::Accent(), 1.5f);
    // Horizontal
    g.DrawLine(&shadowPen, mx, crossCY, mx + MAG_SIZE, crossCY);
    g.DrawLine(&crossPen,  mx, crossCY, mx + MAG_SIZE, crossCY);
    // Vertical
    g.DrawLine(&shadowPen, crossCX, my, crossCX, my + MAG_SIZE);
    g.DrawLine(&crossPen,  crossCX, my, crossCX, my + MAG_SIZE);

    // Highlight the center pixel cell
    Gdiplus::Pen cellPen(Gdiplus::Color(200, 255, 255, 255), 1.0f);
    g.DrawRectangle(&cellPen, crossCX - halfCell, crossCY - halfCell, MAG_ZOOM, MAG_ZOOM);

    DeleteObject(hMag);
}
