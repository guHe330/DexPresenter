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

#include "SettingsDialog.h"
#include "App.h"
#include "Constants.h"
#include "Version.h"
#include <commctrl.h>
#include <prsht.h>
#include <shobjidl.h>
#include <vector>
#include <string>

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

    void AddButton(const wchar_t* text, int id, int x, int y, int w, int h) {
        AddControl(text, id, 0x0080, x, y, w, h,
            BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP);
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
#define IDC_HOTKEY_CTRL           1001
#define IDC_ZOOM_LEVEL            1002
#define IDC_PEN_WIDTH             1003
#define IDC_TIMER_MINUTES         1004
#define IDC_AUTOSTART             1005
#define IDC_SCREENSHOT_DIR        1006
#define IDC_SCREENSHOT_DIR_BROWSE 1007
#define IDC_LABEL1                1010
#define IDC_LABEL2                1011
#define IDC_LABEL3                1012

// Show a folder picker. Returns true if the user chose a folder.
static bool PickFolder(HWND owner, std::wstring& outPath) {
    IFileOpenDialog* dialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&dialog)))) {
        return false;
    }

    DWORD opts = 0;
    dialog->GetOptions(&opts);
    dialog->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
    dialog->SetTitle(L"Choose screenshot folder");

    bool ok = false;
    if (SUCCEEDED(dialog->Show(owner))) {
        IShellItem* item = nullptr;
        if (SUCCEEDED(dialog->GetResult(&item))) {
            PWSTR path = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                outPath = path;
                CoTaskMemFree(path);
                ok = true;
            }
            item->Release();
        }
    }
    dialog->Release();
    return ok;
}


// --- General Page ---
INT_PTR CALLBACK SettingsDialog::GeneralProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        CheckDlgButton(hwnd, IDC_AUTOSTART,
            s_app->GetConfig().startWithWindows ? BST_CHECKED : BST_UNCHECKED);
        std::wstring dir = Utf8ToWide(s_app->GetConfig().screenshotDir);
        SetDlgItemTextW(hwnd, IDC_SCREENSHOT_DIR, dir.c_str());
        return TRUE;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == IDC_SCREENSHOT_DIR_BROWSE && HIWORD(wParam) == BN_CLICKED) {
            std::wstring picked;
            if (PickFolder(hwnd, picked)) {
                SetDlgItemTextW(hwnd, IDC_SCREENSHOT_DIR, picked.c_str());
            }
            return TRUE;
        }
        break;
    }
    case WM_NOTIFY: {
        auto nmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (nmhdr->code == PSN_APPLY) {
            s_app->GetConfig().startWithWindows = (IsDlgButtonChecked(hwnd, IDC_AUTOSTART) == BST_CHECKED);
            wchar_t buf[MAX_PATH];
            GetDlgItemTextW(hwnd, IDC_SCREENSHOT_DIR, buf, MAX_PATH);
            // Trim trailing whitespace/slashes
            std::wstring w = buf;
            while (!w.empty() && (w.back() == L' ' || w.back() == L'\t' ||
                                  w.back() == L'\\' || w.back() == L'/')) {
                w.pop_back();
            }
            s_app->GetConfig().screenshotDir = WideToUtf8(w);
            s_app->SaveConfig();
            s_app->ApplyAutostart(); // create/remove the autorun registry entry now
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

// --- Zoom Page ---
INT_PTR CALLBACK SettingsDialog::ZoomProc(HWND hwnd, UINT msg, WPARAM /*wParam*/, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        auto& cfg = s_app->GetConfig().zoom;
        wchar_t buf[16];
        swprintf_s(buf, L"%.1f", cfg.defaultZoomLevel);
        SetDlgItemTextW(hwnd, IDC_ZOOM_LEVEL, buf);
        return TRUE;
    }
    case WM_NOTIFY: {
        auto nmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (nmhdr->code == PSN_APPLY) {
            auto& cfg = s_app->GetConfig().zoom;
            wchar_t buf[16];
            GetDlgItemTextW(hwnd, IDC_ZOOM_LEVEL, buf, 16);
            float level = (float)_wtof(buf);
            if (level >= 1.1f && level <= 10.0f) cfg.defaultZoomLevel = level;
            s_app->SaveConfig();
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

// --- Timer Page ---
INT_PTR CALLBACK SettingsDialog::TimerProc(HWND hwnd, UINT msg, WPARAM /*wParam*/, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        auto& cfg = s_app->GetConfig().timer;
        SetDlgItemInt(hwnd, IDC_TIMER_MINUTES, cfg.durationSeconds / 60, FALSE);
        return TRUE;
    }
    case WM_NOTIFY: {
        auto nmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (nmhdr->code == PSN_APPLY) {
            auto& cfg = s_app->GetConfig().timer;
            int mins = GetDlgItemInt(hwnd, IDC_TIMER_MINUTES, nullptr, FALSE);
            if (mins >= 1 && mins <= 999) cfg.durationSeconds = mins * 60;
            s_app->SaveConfig();
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

// DrawProc and SnipProc retained for re-enable later — see SettingsDialog.h.
INT_PTR CALLBACK SettingsDialog::DrawProc(HWND, UINT, WPARAM, LPARAM) { return FALSE; }
INT_PTR CALLBACK SettingsDialog::SnipProc(HWND, UINT, WPARAM, LPARAM) { return FALSE; }

void SettingsDialog::Show(App* app, HINSTANCE hInstance, int initialTab) {
    s_app = app;

    // Build dialog templates in memory
    // General page
    DialogTemplate genDlg;
    genDlg.Create(L"General", 260, 120);
    genDlg.AddCheckbox(L"Start with Windows", IDC_AUTOSTART, 10, 12, 200, 14);
    genDlg.AddStatic(L"Screenshot folder:", IDC_LABEL1, 10, 38, 90, 10);
    genDlg.AddStatic(L"(leave blank for default)", IDC_LABEL2, 105, 38, 145, 10);
    genDlg.AddEdit(IDC_SCREENSHOT_DIR, 10, 52, 185, 14);
    genDlg.AddButton(L"Browse...", IDC_SCREENSHOT_DIR_BROWSE, 200, 51, 50, 16);

    // Zoom page
    DialogTemplate zoomDlg;
    zoomDlg.Create(L"Zoom", 220, 120);
    zoomDlg.AddStatic(L"Default zoom level:", IDC_LABEL2, 10, 17, 80, 12);
    zoomDlg.AddEdit(IDC_ZOOM_LEVEL, 100, 15, 40, 14);

    // Timer page
    DialogTemplate timerDlg;
    timerDlg.Create(L"Timer", 220, 120);
    timerDlg.AddStatic(L"Duration (minutes):", IDC_LABEL2, 10, 17, 80, 12);
    timerDlg.AddEdit(IDC_TIMER_MINUTES, 100, 15, 40, 14, ES_NUMBER);

    PROPSHEETPAGEW pages[3] = {};

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
    pages[2].pResource = timerDlg.Get();
    pages[2].pfnDlgProc = TimerProc;

    PROPSHEETHEADERW psh = {};
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hInstance = hInstance;
    psh.pszCaption = L"DexPresenter Settings";
    psh.nPages = 3;
    psh.nStartPage = (initialTab >= 0 && initialTab < 3) ? initialTab : 0;
    psh.ppsp = pages;

    PropertySheetW(&psh);
}
