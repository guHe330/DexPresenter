#include "SettingsDialog.h"
#include "App.h"
#include "Constants.h"
#include "Version.h"
#include <commctrl.h>
#include <prsht.h>
#include <vector>

#pragma comment(lib, "comctl32.lib")

App* SettingsDialog::s_app = nullptr;

// Helper: build an in-memory dialog template (no .rc needed)
struct DialogTemplate {
    std::vector<BYTE> data;

    void Create(const wchar_t* title, int width, int height) {
        // DLGTEMPLATE
        Align(4);
        DLGTEMPLATE dt = {};
        dt.style = DS_SETFONT | WS_CHILD | WS_DISABLED | WS_CAPTION;
        dt.cx = (SHORT)width;
        dt.cy = (SHORT)height;
        Append(&dt, sizeof(dt));
        AppendWord(0); // menu
        AppendWord(0); // class
        AppendString(title);
        AppendWord(9); // font size
        AppendString(L"Segoe UI"); // font name
    }

    void AddStatic(const wchar_t* text, int id, int x, int y, int w, int h) {
        AddControl(text, id, 0x0082, x, y, w, h, SS_LEFT | WS_CHILD | WS_VISIBLE);
    }

    void AddEdit(int id, int x, int y, int w, int h, DWORD extraStyle = 0) {
        AddControl(L"", id, 0x0081, x, y, w, h,
            ES_LEFT | ES_AUTOHSCROLL | WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP | extraStyle);
    }

    void AddCheckbox(const wchar_t* text, int id, int x, int y, int w, int h) {
        AddControl(text, id, 0x0080, x, y, w, h,
            BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP);
    }

    void AddHotkey(int id, int x, int y, int w, int h) {
        Align(4);
        DLGITEMTEMPLATE item = {};
        item.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
        item.x = (SHORT)x;
        item.y = (SHORT)y;
        item.cx = (SHORT)w;
        item.cy = (SHORT)h;
        item.id = (WORD)id;
        Append(&item, sizeof(item));
        AppendString(HOTKEY_CLASSW);  // window class
        AppendString(L"");             // title
        AppendWord(0);                 // extra data
        // Update control count
        auto dt = reinterpret_cast<DLGTEMPLATE*>(data.data());
        dt->cdit++;
    }

    DLGTEMPLATE* Get() { return reinterpret_cast<DLGTEMPLATE*>(data.data()); }

private:
    void AddControl(const wchar_t* text, int id, WORD cls, int x, int y, int w, int h, DWORD style) {
        Align(4);
        DLGITEMTEMPLATE item = {};
        item.style = style;
        item.x = (SHORT)x;
        item.y = (SHORT)y;
        item.cx = (SHORT)w;
        item.cy = (SHORT)h;
        item.id = (WORD)id;
        Append(&item, sizeof(item));
        AppendWord(0xFFFF); // class is predefined
        AppendWord(cls);
        AppendString(text);
        AppendWord(0); // extra data
        auto dt = reinterpret_cast<DLGTEMPLATE*>(data.data());
        dt->cdit++;
    }

    void Align(int n) {
        while (data.size() % n) data.push_back(0);
    }
    void Append(const void* p, size_t sz) {
        auto b = reinterpret_cast<const BYTE*>(p);
        data.insert(data.end(), b, b + sz);
    }
    void AppendWord(WORD w) { Append(&w, sizeof(w)); }
    void AppendString(const wchar_t* s) {
        size_t len = wcslen(s) + 1;
        Append(s, len * sizeof(wchar_t));
    }
};

// Control IDs
#define IDC_HOTKEY_CTRL    1001
#define IDC_ZOOM_LEVEL     1002
#define IDC_PEN_WIDTH      1003
#define IDC_TIMER_MINUTES  1004
#define IDC_AUTOSTART      1005
#define IDC_LABEL1         1010
#define IDC_LABEL2         1011


// --- General Page ---
INT_PTR CALLBACK SettingsDialog::GeneralProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        CheckDlgButton(hwnd, IDC_AUTOSTART,
            s_app->GetConfig().startWithWindows ? BST_CHECKED : BST_UNCHECKED);
        return TRUE;
    }
    case WM_NOTIFY: {
        auto nmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (nmhdr->code == PSN_APPLY) {
            s_app->GetConfig().startWithWindows = (IsDlgButtonChecked(hwnd, IDC_AUTOSTART) == BST_CHECKED);
            s_app->SaveConfig();
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

// --- Zoom Page ---
INT_PTR CALLBACK SettingsDialog::ZoomProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        auto& cfg = s_app->GetConfig().zoom;
        SendDlgItemMessageW(hwnd, IDC_HOTKEY_CTRL, HKM_SETHOTKEY,
            MAKEWORD(cfg.hotkey.vkey, (cfg.hotkey.modifiers & MOD_SHIFT ? HOTKEYF_SHIFT : 0) |
                                       (cfg.hotkey.modifiers & MOD_CONTROL ? HOTKEYF_CONTROL : 0) |
                                       (cfg.hotkey.modifiers & MOD_ALT ? HOTKEYF_ALT : 0)), 0);
        wchar_t buf[16];
        swprintf_s(buf, L"%.1f", cfg.defaultZoomLevel);
        SetDlgItemTextW(hwnd, IDC_ZOOM_LEVEL, buf);
        return TRUE;
    }
    case WM_NOTIFY: {
        auto nmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (nmhdr->code == PSN_APPLY) {
            auto& cfg = s_app->GetConfig().zoom;
            WORD hk = (WORD)SendDlgItemMessageW(hwnd, IDC_HOTKEY_CTRL, HKM_GETHOTKEY, 0, 0);
            BYTE vk = LOBYTE(hk);
            BYTE mods = HIBYTE(hk);
            if (vk) {
                cfg.hotkey.vkey = vk;
                cfg.hotkey.modifiers = 0;
                if (mods & HOTKEYF_CONTROL) cfg.hotkey.modifiers |= MOD_CONTROL;
                if (mods & HOTKEYF_SHIFT)   cfg.hotkey.modifiers |= MOD_SHIFT;
                if (mods & HOTKEYF_ALT)     cfg.hotkey.modifiers |= MOD_ALT;
            }
            wchar_t buf[16];
            GetDlgItemTextW(hwnd, IDC_ZOOM_LEVEL, buf, 16);
            float level = (float)_wtof(buf);
            if (level >= 1.1f && level <= 10.0f) cfg.defaultZoomLevel = level;
            s_app->SaveConfig();
            s_app->ReregisterHotkeys();
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

// --- Draw Page ---
INT_PTR CALLBACK SettingsDialog::DrawProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        auto& cfg = s_app->GetConfig().draw;
        SendDlgItemMessageW(hwnd, IDC_HOTKEY_CTRL, HKM_SETHOTKEY,
            MAKEWORD(cfg.hotkey.vkey, (cfg.hotkey.modifiers & MOD_SHIFT ? HOTKEYF_SHIFT : 0) |
                                       (cfg.hotkey.modifiers & MOD_CONTROL ? HOTKEYF_CONTROL : 0) |
                                       (cfg.hotkey.modifiers & MOD_ALT ? HOTKEYF_ALT : 0)), 0);
        return TRUE;
    }
    case WM_NOTIFY: {
        auto nmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (nmhdr->code == PSN_APPLY) {
            auto& cfg = s_app->GetConfig().draw;
            WORD hk = (WORD)SendDlgItemMessageW(hwnd, IDC_HOTKEY_CTRL, HKM_GETHOTKEY, 0, 0);
            BYTE vk = LOBYTE(hk);
            BYTE mods = HIBYTE(hk);
            if (vk) {
                cfg.hotkey.vkey = vk;
                cfg.hotkey.modifiers = 0;
                if (mods & HOTKEYF_CONTROL) cfg.hotkey.modifiers |= MOD_CONTROL;
                if (mods & HOTKEYF_SHIFT)   cfg.hotkey.modifiers |= MOD_SHIFT;
                if (mods & HOTKEYF_ALT)     cfg.hotkey.modifiers |= MOD_ALT;
            }
            s_app->SaveConfig();
            s_app->ReregisterHotkeys();
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

// --- Timer Page ---
INT_PTR CALLBACK SettingsDialog::TimerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        auto& cfg = s_app->GetConfig().timer;
        SendDlgItemMessageW(hwnd, IDC_HOTKEY_CTRL, HKM_SETHOTKEY,
            MAKEWORD(cfg.hotkey.vkey, (cfg.hotkey.modifiers & MOD_SHIFT ? HOTKEYF_SHIFT : 0) |
                                       (cfg.hotkey.modifiers & MOD_CONTROL ? HOTKEYF_CONTROL : 0) |
                                       (cfg.hotkey.modifiers & MOD_ALT ? HOTKEYF_ALT : 0)), 0);
        SetDlgItemInt(hwnd, IDC_TIMER_MINUTES, cfg.durationSeconds / 60, FALSE);
        return TRUE;
    }
    case WM_NOTIFY: {
        auto nmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (nmhdr->code == PSN_APPLY) {
            auto& cfg = s_app->GetConfig().timer;
            WORD hk = (WORD)SendDlgItemMessageW(hwnd, IDC_HOTKEY_CTRL, HKM_GETHOTKEY, 0, 0);
            BYTE vk = LOBYTE(hk);
            BYTE mods = HIBYTE(hk);
            if (vk) {
                cfg.hotkey.vkey = vk;
                cfg.hotkey.modifiers = 0;
                if (mods & HOTKEYF_CONTROL) cfg.hotkey.modifiers |= MOD_CONTROL;
                if (mods & HOTKEYF_SHIFT)   cfg.hotkey.modifiers |= MOD_SHIFT;
                if (mods & HOTKEYF_ALT)     cfg.hotkey.modifiers |= MOD_ALT;
            }
            int mins = GetDlgItemInt(hwnd, IDC_TIMER_MINUTES, nullptr, FALSE);
            if (mins >= 1 && mins <= 999) cfg.durationSeconds = mins * 60;
            s_app->SaveConfig();
            s_app->ReregisterHotkeys();
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

// --- Snip Page ---
INT_PTR CALLBACK SettingsDialog::SnipProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        auto& cfg = s_app->GetConfig().snip;
        SendDlgItemMessageW(hwnd, IDC_HOTKEY_CTRL, HKM_SETHOTKEY,
            MAKEWORD(cfg.hotkey.vkey, (cfg.hotkey.modifiers & MOD_SHIFT ? HOTKEYF_SHIFT : 0) |
                                       (cfg.hotkey.modifiers & MOD_CONTROL ? HOTKEYF_CONTROL : 0) |
                                       (cfg.hotkey.modifiers & MOD_ALT ? HOTKEYF_ALT : 0)), 0);
        return TRUE;
    }
    case WM_NOTIFY: {
        auto nmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (nmhdr->code == PSN_APPLY) {
            auto& cfg = s_app->GetConfig().snip;
            WORD hk = (WORD)SendDlgItemMessageW(hwnd, IDC_HOTKEY_CTRL, HKM_GETHOTKEY, 0, 0);
            BYTE vk = LOBYTE(hk);
            BYTE mods = HIBYTE(hk);
            if (vk) {
                cfg.hotkey.vkey = vk;
                cfg.hotkey.modifiers = 0;
                if (mods & HOTKEYF_CONTROL) cfg.hotkey.modifiers |= MOD_CONTROL;
                if (mods & HOTKEYF_SHIFT)   cfg.hotkey.modifiers |= MOD_SHIFT;
                if (mods & HOTKEYF_ALT)     cfg.hotkey.modifiers |= MOD_ALT;
            }
            s_app->SaveConfig();
            s_app->ReregisterHotkeys();
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

void SettingsDialog::Show(App* app, HINSTANCE hInstance, int initialTab) {
    s_app = app;

    // Build dialog templates in memory
    // General page
    DialogTemplate genDlg;
    genDlg.Create(L"General", 220, 120);
    genDlg.AddCheckbox(L"Start with Windows", IDC_AUTOSTART, 10, 15, 180, 14);

    // Zoom page
    DialogTemplate zoomDlg;
    zoomDlg.Create(L"Zoom", 220, 120);
    zoomDlg.AddStatic(L"Hotkey:", IDC_LABEL1, 10, 17, 50, 12);
    zoomDlg.AddHotkey(IDC_HOTKEY_CTRL, 65, 14, 130, 14);
    zoomDlg.AddStatic(L"Default zoom level:", IDC_LABEL2, 10, 40, 80, 12);
    zoomDlg.AddEdit(IDC_ZOOM_LEVEL, 100, 38, 40, 14);

    // Draw page — just hotkey (colors/widths configured via right-click menu in overlay)
    DialogTemplate drawDlg;
    drawDlg.Create(L"Draw", 220, 120);
    drawDlg.AddStatic(L"Hotkey:", IDC_LABEL1, 10, 17, 50, 12);
    drawDlg.AddHotkey(IDC_HOTKEY_CTRL, 65, 14, 130, 14);

    // Timer page
    DialogTemplate timerDlg;
    timerDlg.Create(L"Timer", 220, 120);
    timerDlg.AddStatic(L"Hotkey:", IDC_LABEL1, 10, 17, 50, 12);
    timerDlg.AddHotkey(IDC_HOTKEY_CTRL, 65, 14, 130, 14);
    timerDlg.AddStatic(L"Duration (minutes):", IDC_LABEL2, 10, 40, 80, 12);
    timerDlg.AddEdit(IDC_TIMER_MINUTES, 100, 38, 40, 14, ES_NUMBER);

    // Snip page
    DialogTemplate snipDlg;
    snipDlg.Create(L"Snip", 220, 120);
    snipDlg.AddStatic(L"Hotkey:", IDC_LABEL1, 10, 17, 50, 12);
    snipDlg.AddHotkey(IDC_HOTKEY_CTRL, 65, 14, 130, 14);

    PROPSHEETPAGEW pages[5] = {};

    pages[0].dwSize = sizeof(PROPSHEETPAGEW);
    pages[0].dwFlags = PSP_DLGINDIRECT;
    pages[0].hInstance = hInstance;
    pages[0].pResource = genDlg.Get();
    pages[0].pfnDlgProc = GeneralProc;

    pages[1].dwSize = sizeof(PROPSHEETPAGEW);
    pages[1].dwFlags = PSP_DLGINDIRECT;
    pages[1].hInstance = hInstance;
    pages[1].pResource = zoomDlg.Get();
    pages[1].pfnDlgProc = ZoomProc;

    pages[2].dwSize = sizeof(PROPSHEETPAGEW);
    pages[2].dwFlags = PSP_DLGINDIRECT;
    pages[2].hInstance = hInstance;
    pages[2].pResource = drawDlg.Get();
    pages[2].pfnDlgProc = DrawProc;

    pages[3].dwSize = sizeof(PROPSHEETPAGEW);
    pages[3].dwFlags = PSP_DLGINDIRECT;
    pages[3].hInstance = hInstance;
    pages[3].pResource = timerDlg.Get();
    pages[3].pfnDlgProc = TimerProc;

    pages[4].dwSize = sizeof(PROPSHEETPAGEW);
    pages[4].dwFlags = PSP_DLGINDIRECT;
    pages[4].hInstance = hInstance;
    pages[4].pResource = snipDlg.Get();
    pages[4].pfnDlgProc = SnipProc;

    PROPSHEETHEADERW psh = {};
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hInstance = hInstance;
    psh.pszCaption = L"DexPresenter Settings";
    psh.nPages = 5;
    psh.nStartPage = (initialTab >= 0 && initialTab < 5) ? initialTab : 0;
    psh.ppsp = pages;

    PropertySheetW(&psh);
}
