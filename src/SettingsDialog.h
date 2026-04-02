#pragma once
#include <Windows.h>

class App;

class SettingsDialog {
public:
    static void Show(App* app, HINSTANCE hInstance, int initialTab = 0);

private:
    static INT_PTR CALLBACK GeneralProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK ZoomProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK DrawProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK TimerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK SnipProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static App* s_app;
};
