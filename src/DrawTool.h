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
#include <vector>

enum class ShapeMode { Freehand, Arrow, Box, Circle, Obfuscate, Eraser };

struct DrawElement {
    ShapeMode type = ShapeMode::Freehand;
    std::vector<POINT> points;  // For freehand
    POINT startPt = {};         // For shapes
    POINT endPt = {};           // For shapes
    COLORREF color = RGB(255, 0, 0);
    int width = 3;
    mutable HBITMAP hObfuscateCache = nullptr;  // cached render for Obfuscate type

    ~DrawElement() { if (hObfuscateCache) { DeleteObject(hObfuscateCache); hObfuscateCache = nullptr; } }
    DrawElement() = default;
    // Copy: don't copy the cache (it will be recomputed on next draw)
    DrawElement(const DrawElement& o)
        : type(o.type), points(o.points), startPt(o.startPt), endPt(o.endPt),
          color(o.color), width(o.width) {}
    DrawElement& operator=(const DrawElement& o) {
        if (this != &o) {
            if (hObfuscateCache) { DeleteObject(hObfuscateCache); hObfuscateCache = nullptr; }
            type = o.type; points = o.points; startPt = o.startPt; endPt = o.endPt;
            color = o.color; width = o.width;
        }
        return *this;
    }
    // Move: transfer the cache
    DrawElement(DrawElement&& o) noexcept
        : type(o.type), points(std::move(o.points)), startPt(o.startPt), endPt(o.endPt),
          color(o.color), width(o.width), hObfuscateCache(o.hObfuscateCache) {
        o.hObfuscateCache = nullptr;
    }
    DrawElement& operator=(DrawElement&& o) noexcept {
        if (this != &o) {
            if (hObfuscateCache) DeleteObject(hObfuscateCache);
            type = o.type; points = std::move(o.points); startPt = o.startPt; endPt = o.endPt;
            color = o.color; width = o.width; hObfuscateCache = o.hObfuscateCache;
            o.hObfuscateCache = nullptr;
        }
        return *this;
    }
};

struct UndoAction {
    enum Type { Draw, Erase };
    Type type;
    int erasedIndex = 0;            // For Erase: original index in m_elements
    ::DrawElement erasedElement;    // For Erase: the removed element
};

class DrawTool : public OverlayWindow {
public:
    void SetConfig(const DrawConfig& cfg) { m_config = cfg; }
    const DrawConfig& GetConfig() const { return m_config; }
    void SetScreenshotDir(const std::string& dir) { m_screenshotDir = dir; }

protected:
    const wchar_t* ClassName() const override { return L"DexPresenterDraw"; }
    void OnPaint(HDC hdc, int width, int height) override;
    void OnKeyDown(WPARAM vkey) override;
    void OnMouseMove(int x, int y) override;
    void OnMouseWheel(int delta, bool ctrlHeld) override;
    void OnLButtonDown(int x, int y) override;
    void OnLButtonUp(int x, int y) override;
    void OnRButtonDown(int x, int y) override;
    void OnTimer(UINT_PTR id) override;
    void OnActivated() override;
    void OnDismissed() override;

private:
    void DrawAllElements(Gdiplus::Graphics& graphics);
    void DrawElement(Gdiplus::Graphics& graphics, const ::DrawElement& elem, bool useCache = false);
    void DrawArrowShape(Gdiplus::Graphics& graphics, POINT start, POINT end, COLORREF color, int width);
    void DrawSmoothCurve(Gdiplus::Graphics& graphics, const ::DrawElement& elem);
    int EraserHitTest(int x, int y) const;
    void Undo();
    void Redo();
    void ClearAll();
    void SaveScreenshot();
    HBITMAP CompositeFinalImage();

    // Context menu
    void ShowContextMenu(int x, int y);
    void PaintContextMenu(HDC hdc);
    int ContextMenuHitTest(int x, int y);
    void CloseContextMenu();

    // Per-shape style helpers
    const ShapeStyle& GetCurrentStyle() const;
    ShapeStyle& GetMutableStyle();
    void ApplyCurrentStyle();
    void SaveCurrentStyle();

    HBITMAP m_hScreen = nullptr;
    DrawConfig m_config;
    std::string m_screenshotDir;
    COLORREF m_penColor = RGB(255, 0, 0);
    int m_penWidth = 3;
    bool m_drawing = false;
    ShapeMode m_shapeMode = ShapeMode::Freehand;

    std::vector<::DrawElement> m_elements;
    std::vector<UndoAction> m_undoStack;
    std::vector<UndoAction> m_redoStack;
    ::DrawElement m_currentElement;

    // Smooth mode: exponential smoothing state
    float m_smoothX = 0, m_smoothY = 0;  // running smoothed position
    static constexpr float SMOOTH_ALPHA = 0.25f;  // lower = smoother (0.1 very smooth, 0.5 mild)
    static constexpr float SMOOTH_MIN_DIST = 4.0f; // min pixel distance to add a point

    // Context menu state
    bool m_menuVisible = false;
    int m_menuX = 0, m_menuY = 0;
    int m_menuHover = -1;

    // Menu layout constants
    static constexpr int MENU_ITEM_HEIGHT = 32;
    static constexpr int MENU_ITEM_WIDTH = 180;
    static constexpr int MENU_PADDING = 6;
    static constexpr int MENU_SEPARATOR_HEIGHT = 9;
    static constexpr int MENU_SWATCH_SIZE = 24;
    static constexpr int MENU_SWATCH_GAP = 4;
    static constexpr int MENU_SWATCH_COLS = 4;
    static constexpr int MENU_SWATCH_ROWS = 2;
    static constexpr int MENU_WIDTH_DOT_SIZE = 24;

    // Menu item indices
    // 0-3: shape modes, 4: separator, 5-12: colors, 13: separator, 14-17: widths
    int GetMenuTotalHeight() const;

    static constexpr UINT_PTR HINT_TIMER_ID  = 200;
    static constexpr int HINT_VISIBLE_MS     = 2000;
    static constexpr int HINT_FADE_MS        = 500;
    static constexpr int HINT_TICK_MS        = 50;

    int  m_hintElapsedMs = 0;
    bool m_hintVisible   = false;
};
