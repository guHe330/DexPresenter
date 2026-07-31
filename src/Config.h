/**
 * DexPresenter - a free and open source Windows presentation utility with drawing tools
 * Copyright (C) 2026 Gunter Heiss
 *
 * For more information see: https://dexpresenter.com
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
#include <Windows.h>
#include <filesystem>
#include <string>
#include <map>

struct HotkeyConfig {
    int modifiers = MOD_CONTROL;
    int vkey = 0;
};

struct ZoomConfig {
    HotkeyConfig hotkey = { MOD_CONTROL, '1' };
    float defaultZoomLevel = 2.0f;
};

struct ShapeStyle {
    int colorR = 255, colorG = 0, colorB = 0;
    int width = 3;
};

struct DrawConfig {
    HotkeyConfig hotkey = { MOD_CONTROL, '2' };
    // Theme accent #E64A81. Kept as literals rather than Theme:: constants so
    // this header stays free of the GDI+ include (Config.cpp builds into the
    // test binary, which does not link GDI+). Keep in sync with Constants.h.
    ShapeStyle freehand = { 230, 74, 129, 3 };
    ShapeStyle arrow    = { 230, 74, 129, 3 };
    ShapeStyle box      = { 0, 200, 0, 3 };
    ShapeStyle circle   = { 0, 0, 255, 3 };
    int lastMode = 0;       // ShapeMode enum value (persisted between sessions)
    int lastColorR = 255, lastColorG = 0, lastColorB = 0;
    int lastWidth = 3;
};

struct TimerConfig {
    HotkeyConfig hotkey = { MOD_CONTROL, '3' };
    int durationSeconds = 600;
};

struct SnipConfig {
    HotkeyConfig hotkey = { MOD_CONTROL, '6' };
};

struct ToolbarConfig {
    int posX = -1;  // -1 means center at top edge
    int posY = 0;   // default: top edge of screen
    bool visible = true;
};

struct AppConfig {
    ZoomConfig zoom;
    DrawConfig draw;
    TimerConfig timer;
    SnipConfig snip;
    ToolbarConfig toolbar;
    bool startWithWindows = false;
    std::string screenshotDir; // empty = default (%USERPROFILE%\Pictures\DexPresenter)
};

// Utility: format seconds as MM:SS or H:MM:SS
std::wstring FormatTime(int totalSeconds);

// Convert UTF-8 std::string to std::wstring
std::wstring Utf8ToWide(const std::string& str);
// Convert std::wstring to UTF-8 std::string
std::string WideToUtf8(const std::wstring& str);

// Simple INI-style config file reader/writer
class IniFile {
public:
    void Set(const std::string& section, const std::string& key, const std::string& value);
    std::string Get(const std::string& section, const std::string& key, const std::string& defaultVal = "") const;
    int GetInt(const std::string& section, const std::string& key, int defaultVal = 0) const;
    float GetFloat(const std::string& section, const std::string& key, float defaultVal = 0.0f) const;
    bool GetBool(const std::string& section, const std::string& key, bool defaultVal = false) const;

    void SetInt(const std::string& section, const std::string& key, int value);
    void SetFloat(const std::string& section, const std::string& key, float value);
    void SetBool(const std::string& section, const std::string& key, bool value);

    // std::filesystem::path keeps the path wide on Windows, so config files in
    // profiles with non-ANSI characters (e.g. C:\Users\Müller) work correctly.
    bool LoadFromFile(const std::filesystem::path& path);
    bool SaveToFile(const std::filesystem::path& path) const;

    // For testing: load/save to string
    void LoadFromString(const std::string& content);
    std::string SaveToString() const;

private:
    // section -> (key -> value)
    std::map<std::string, std::map<std::string, std::string>> m_data;
};

// Serialize/deserialize AppConfig to/from IniFile
void AppConfigToIni(const AppConfig& config, IniFile& ini);
AppConfig AppConfigFromIni(const IniFile& ini);

class Config {
public:
    static std::wstring GetConfigDir();
    static std::filesystem::path GetConfigPath();
    static AppConfig    Load();
    static void         Save(const AppConfig& config);
};
