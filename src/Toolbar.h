#pragma once
#include <Windows.h>
#include <string>

class App;

class Toolbar {
public:
    Toolbar(App* app, HINSTANCE hInstance);
    ~Toolbar();

    void Show();
    void Hide();
    bool IsVisible() const;
    void SavePosition();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void Paint(HDC hdc);
    int HitTest(int x, int y);
    std::wstring GetButtonTooltip(int index);

    App* m_app;
    HINSTANCE m_hInstance;
    HWND m_hwnd = nullptr;
    HWND m_hwndTooltip = nullptr;
    int m_hoverIndex = -1;
    bool m_classRegistered = false;

    // Button icons (drawn procedurally)
    static constexpr int NUM_BUTTONS = 5; // Snip, Draw, Timer, Zoom, Close
};
