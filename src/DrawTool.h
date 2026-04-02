#pragma once
#include "OverlayWindow.h"
#include "Config.h"
#include <vector>

enum class ShapeMode { Freehand, Smooth, Arrow, Box, Circle, Obfuscate, Eraser };

struct DrawElement {
    ShapeMode type = ShapeMode::Freehand;
    std::vector<POINT> points;  // For freehand
    POINT startPt = {};         // For shapes
    POINT endPt = {};           // For shapes
    COLORREF color = RGB(255, 0, 0);
    int width = 3;
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

protected:
    const wchar_t* ClassName() const override { return L"DexPresenterDraw"; }
    void OnPaint(HDC hdc, int width, int height) override;
    void OnKeyDown(WPARAM vkey) override;
    void OnMouseMove(int x, int y) override;
    void OnMouseWheel(int delta, bool ctrlHeld) override;
    void OnLButtonDown(int x, int y) override;
    void OnLButtonUp(int x, int y) override;
    void OnRButtonDown(int x, int y) override;
    void OnActivated() override;
    void OnDismissed() override;

private:
    void DrawAllElements(Gdiplus::Graphics& graphics);
    void DrawElement(Gdiplus::Graphics& graphics, const ::DrawElement& elem);
    void DrawArrowShape(Gdiplus::Graphics& graphics, POINT start, POINT end, COLORREF color, int width);
    void DrawSmoothCurve(Gdiplus::Graphics& graphics, const ::DrawElement& elem);
    int EraserHitTest(int x, int y) const;
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
};
