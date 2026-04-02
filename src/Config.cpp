#include "Config.h"
#include <ShlObj.h>
#include <fstream>
#include <sstream>
#include <filesystem>

std::wstring FormatTime(int totalSeconds) {
    if (totalSeconds < 0) totalSeconds = 0;
    int h = totalSeconds / 3600;
    int m = (totalSeconds % 3600) / 60;
    int s = totalSeconds % 60;
    wchar_t buf[32];
    if (h > 0)
        swprintf_s(buf, L"%d:%02d:%02d", h, m, s);
    else
        swprintf_s(buf, L"%02d:%02d", m, s);
    return buf;
}

std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr.data(), len);
    return wstr;
}

std::string WideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, str.data(), len, nullptr, nullptr);
    return str;
}

// ============================================================================
// IniFile implementation
// ============================================================================

static std::string Trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

void IniFile::Set(const std::string& section, const std::string& key, const std::string& value) {
    m_data[section][key] = value;
}

std::string IniFile::Get(const std::string& section, const std::string& key, const std::string& defaultVal) const {
    auto sit = m_data.find(section);
    if (sit == m_data.end()) return defaultVal;
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) return defaultVal;
    return kit->second;
}

int IniFile::GetInt(const std::string& section, const std::string& key, int defaultVal) const {
    std::string val = Get(section, key, "");
    if (val.empty()) return defaultVal;
    try { return std::stoi(val); } catch (...) { return defaultVal; }
}

float IniFile::GetFloat(const std::string& section, const std::string& key, float defaultVal) const {
    std::string val = Get(section, key, "");
    if (val.empty()) return defaultVal;
    try { return std::stof(val); } catch (...) { return defaultVal; }
}

bool IniFile::GetBool(const std::string& section, const std::string& key, bool defaultVal) const {
    std::string val = Get(section, key, "");
    if (val.empty()) return defaultVal;
    return val == "1" || val == "true" || val == "yes";
}

void IniFile::SetInt(const std::string& section, const std::string& key, int value) {
    Set(section, key, std::to_string(value));
}

void IniFile::SetFloat(const std::string& section, const std::string& key, float value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f", value);
    Set(section, key, buf);
}

void IniFile::SetBool(const std::string& section, const std::string& key, bool value) {
    Set(section, key, value ? "1" : "0");
}

void IniFile::LoadFromString(const std::string& content) {
    m_data.clear();
    std::istringstream stream(content);
    std::string line;
    std::string currentSection;

    while (std::getline(stream, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        if (line.front() == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }

        auto eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = Trim(line.substr(0, eq));
            std::string val = Trim(line.substr(eq + 1));
            if (!key.empty()) {
                m_data[currentSection][key] = val;
            }
        }
    }
}

std::string IniFile::SaveToString() const {
    std::ostringstream out;
    for (auto& [section, kvs] : m_data) {
        out << "[" << section << "]\n";
        for (auto& [key, val] : kvs) {
            out << key << " = " << val << "\n";
        }
        out << "\n";
    }
    return out.str();
}

bool IniFile::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    LoadFromString(content);
    return true;
}

bool IniFile::SaveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << SaveToString();
    return file.good();
}

// ============================================================================
// AppConfig <-> IniFile serialization
// ============================================================================

static void HotkeyToIni(IniFile& ini, const std::string& section,
                         const std::string& prefix, const HotkeyConfig& hk) {
    ini.SetInt(section, prefix + "_modifiers", hk.modifiers);
    ini.SetInt(section, prefix + "_vkey", hk.vkey);
}

static HotkeyConfig HotkeyFromIni(const IniFile& ini, const std::string& section,
                                    const std::string& prefix, const HotkeyConfig& defaults) {
    HotkeyConfig hk;
    hk.modifiers = ini.GetInt(section, prefix + "_modifiers", defaults.modifiers);
    hk.vkey = ini.GetInt(section, prefix + "_vkey", defaults.vkey);
    return hk;
}

void AppConfigToIni(const AppConfig& config, IniFile& ini) {
    // Zoom
    HotkeyToIni(ini, "zoom", "hotkey", config.zoom.hotkey);
    ini.SetFloat("zoom", "defaultZoomLevel", config.zoom.defaultZoomLevel);

    // Draw
    HotkeyToIni(ini, "draw", "hotkey", config.draw.hotkey);
    auto saveStyle = [&](const std::string& prefix, const ShapeStyle& s) {
        ini.SetInt("draw", prefix + "_colorR", s.colorR);
        ini.SetInt("draw", prefix + "_colorG", s.colorG);
        ini.SetInt("draw", prefix + "_colorB", s.colorB);
        ini.SetInt("draw", prefix + "_width", s.width);
    };
    saveStyle("freehand", config.draw.freehand);
    saveStyle("arrow", config.draw.arrow);
    saveStyle("box", config.draw.box);
    saveStyle("circle", config.draw.circle);
    saveStyle("smooth", config.draw.smooth);
    ini.SetInt("draw", "lastMode", config.draw.lastMode);
    ini.SetInt("draw", "lastColorR", config.draw.lastColorR);
    ini.SetInt("draw", "lastColorG", config.draw.lastColorG);
    ini.SetInt("draw", "lastColorB", config.draw.lastColorB);
    ini.SetInt("draw", "lastWidth", config.draw.lastWidth);
    ini.Set("draw", "screenshotDir", config.draw.screenshotDir);

    // Timer
    HotkeyToIni(ini, "timer", "hotkey", config.timer.hotkey);
    ini.SetInt("timer", "durationSeconds", config.timer.durationSeconds);

    // Snip
    HotkeyToIni(ini, "snip", "hotkey", config.snip.hotkey);
    ini.Set("snip", "screenshotDir", config.snip.screenshotDir);

    // Toolbar
    ini.SetInt("toolbar", "posX", config.toolbar.posX);
    ini.SetInt("toolbar", "posY", config.toolbar.posY);
    ini.SetBool("toolbar", "visible", config.toolbar.visible);

    // General
    ini.SetBool("general", "startWithWindows", config.startWithWindows);
}

AppConfig AppConfigFromIni(const IniFile& ini) {
    AppConfig config;

    // Zoom
    config.zoom.hotkey = HotkeyFromIni(ini, "zoom", "hotkey", config.zoom.hotkey);
    config.zoom.defaultZoomLevel = ini.GetFloat("zoom", "defaultZoomLevel", config.zoom.defaultZoomLevel);

    // Draw
    config.draw.hotkey = HotkeyFromIni(ini, "draw", "hotkey", config.draw.hotkey);
    auto loadStyle = [&](const std::string& prefix, ShapeStyle& s) {
        s.colorR = ini.GetInt("draw", prefix + "_colorR", s.colorR);
        s.colorG = ini.GetInt("draw", prefix + "_colorG", s.colorG);
        s.colorB = ini.GetInt("draw", prefix + "_colorB", s.colorB);
        s.width  = ini.GetInt("draw", prefix + "_width", s.width);
    };
    loadStyle("freehand", config.draw.freehand);
    loadStyle("arrow", config.draw.arrow);
    loadStyle("box", config.draw.box);
    loadStyle("circle", config.draw.circle);
    loadStyle("smooth", config.draw.smooth);
    config.draw.lastMode = ini.GetInt("draw", "lastMode", config.draw.lastMode);
    config.draw.lastColorR = ini.GetInt("draw", "lastColorR", config.draw.lastColorR);
    config.draw.lastColorG = ini.GetInt("draw", "lastColorG", config.draw.lastColorG);
    config.draw.lastColorB = ini.GetInt("draw", "lastColorB", config.draw.lastColorB);
    config.draw.lastWidth = ini.GetInt("draw", "lastWidth", config.draw.lastWidth);
    config.draw.screenshotDir = ini.Get("draw", "screenshotDir", config.draw.screenshotDir);

    // Timer
    config.timer.hotkey = HotkeyFromIni(ini, "timer", "hotkey", config.timer.hotkey);
    config.timer.durationSeconds = ini.GetInt("timer", "durationSeconds", config.timer.durationSeconds);

    // Snip
    config.snip.hotkey = HotkeyFromIni(ini, "snip", "hotkey", config.snip.hotkey);
    config.snip.screenshotDir = ini.Get("snip", "screenshotDir", config.snip.screenshotDir);

    // Toolbar
    config.toolbar.posX = ini.GetInt("toolbar", "posX", config.toolbar.posX);
    config.toolbar.posY = ini.GetInt("toolbar", "posY", config.toolbar.posY);
    config.toolbar.visible = ini.GetBool("toolbar", "visible", config.toolbar.visible);

    // General
    config.startWithWindows = ini.GetBool("general", "startWithWindows", config.startWithWindows);

    return config;
}

// ============================================================================
// Config class
// ============================================================================

std::wstring Config::GetConfigDir() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        std::wstring dir = std::wstring(path) + L"\\DexPresenter";
        CreateDirectoryW(dir.c_str(), nullptr);
        return dir;
    }
    return L".";
}

std::string Config::GetConfigPath() {
    std::wstring wdir = GetConfigDir();
    std::wstring wpath = wdir + L"\\config.ini";
    return WideToUtf8(wpath);
}

AppConfig Config::Load() {
    try {
        IniFile ini;
        if (ini.LoadFromFile(GetConfigPath())) {
            return AppConfigFromIni(ini);
        }
    } catch (...) {
    }
    return AppConfig{};
}

void Config::Save(const AppConfig& config) {
    try {
        GetConfigDir();
        IniFile ini;
        AppConfigToIni(config, ini);
        ini.SaveToFile(GetConfigPath());
    } catch (...) {
    }
}
