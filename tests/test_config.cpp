#include <gtest/gtest.h>
#include "Config.h"

TEST(AppConfig, DefaultValues) {
    AppConfig c;
    EXPECT_EQ(c.zoom.hotkey.vkey, '1');
    EXPECT_EQ(c.zoom.hotkey.modifiers, MOD_CONTROL);
    EXPECT_FLOAT_EQ(c.zoom.defaultZoomLevel, 2.0f);
    EXPECT_EQ(c.draw.hotkey.vkey, '2');
    EXPECT_EQ(c.draw.freehand.width, 3);
    EXPECT_EQ(c.draw.freehand.colorR, 255);
    EXPECT_EQ(c.draw.freehand.colorG, 0);
    EXPECT_EQ(c.draw.freehand.colorB, 0);
    EXPECT_EQ(c.timer.hotkey.vkey, '3');
    EXPECT_EQ(c.timer.durationSeconds, 600);
    EXPECT_EQ(c.snip.hotkey.vkey, '6');
    EXPECT_TRUE(c.draw.screenshotDir.empty());
    EXPECT_TRUE(c.snip.screenshotDir.empty());
    EXPECT_FALSE(c.startWithWindows);
    EXPECT_TRUE(c.toolbar.visible);
    EXPECT_EQ(c.toolbar.posX, -1);
}

TEST(AppConfig, IniRoundTrip) {
    AppConfig orig;
    orig.zoom.defaultZoomLevel = 5.0f;
    orig.draw.freehand.width = 10;
    orig.draw.freehand.colorR = 0;
    orig.draw.freehand.colorG = 255;
    orig.draw.freehand.colorB = 128;
    orig.timer.durationSeconds = 300;
    orig.draw.screenshotDir = "C:\\Screenshots";
    orig.snip.screenshotDir = "D:\\Snips";
    orig.startWithWindows = true;
    orig.toolbar.posX = 100;
    orig.toolbar.posY = 200;
    orig.toolbar.visible = false;

    IniFile ini;
    AppConfigToIni(orig, ini);
    AppConfig back = AppConfigFromIni(ini);

    EXPECT_FLOAT_EQ(back.zoom.defaultZoomLevel, 5.0f);
    EXPECT_EQ(back.draw.freehand.width, 10);
    EXPECT_EQ(back.draw.freehand.colorR, 0);
    EXPECT_EQ(back.draw.freehand.colorG, 255);
    EXPECT_EQ(back.draw.freehand.colorB, 128);
    EXPECT_EQ(back.timer.durationSeconds, 300);
    EXPECT_EQ(back.draw.screenshotDir, "C:\\Screenshots");
    EXPECT_EQ(back.snip.screenshotDir, "D:\\Snips");
    EXPECT_TRUE(back.startWithWindows);
    EXPECT_EQ(back.toolbar.posX, 100);
    EXPECT_EQ(back.toolbar.posY, 200);
    EXPECT_FALSE(back.toolbar.visible);
}

TEST(AppConfig, IniStringRoundTrip) {
    AppConfig orig;
    orig.zoom.hotkey.modifiers = MOD_CONTROL | MOD_SHIFT;
    orig.zoom.hotkey.vkey = 'Z';
    orig.zoom.defaultZoomLevel = 3.5f;
    orig.draw.freehand.width = 7;
    orig.timer.durationSeconds = 120;

    IniFile ini;
    AppConfigToIni(orig, ini);
    std::string content = ini.SaveToString();

    // Verify string contains expected sections and values
    EXPECT_NE(content.find("[zoom]"), std::string::npos);
    EXPECT_NE(content.find("[draw]"), std::string::npos);
    EXPECT_NE(content.find("[timer]"), std::string::npos);
    EXPECT_NE(content.find("[toolbar]"), std::string::npos);

    // Parse it back
    IniFile ini2;
    ini2.LoadFromString(content);
    AppConfig back = AppConfigFromIni(ini2);

    EXPECT_EQ(back.zoom.hotkey.modifiers, MOD_CONTROL | MOD_SHIFT);
    EXPECT_EQ(back.zoom.hotkey.vkey, 'Z');
    EXPECT_FLOAT_EQ(back.zoom.defaultZoomLevel, 3.5f);
    EXPECT_EQ(back.draw.freehand.width, 7);
    EXPECT_EQ(back.timer.durationSeconds, 120);
}

TEST(AppConfig, EmptyIniGetDefaults) {
    IniFile ini;
    ini.LoadFromString("");
    AppConfig c = AppConfigFromIni(ini);

    EXPECT_FLOAT_EQ(c.zoom.defaultZoomLevel, 2.0f);
    EXPECT_EQ(c.draw.freehand.width, 3);
    EXPECT_EQ(c.timer.durationSeconds, 600);
    EXPECT_EQ(c.zoom.hotkey.vkey, '1');
    EXPECT_EQ(c.snip.hotkey.vkey, '6');
    EXPECT_TRUE(c.toolbar.visible);
    EXPECT_FALSE(c.startWithWindows);
}

TEST(AppConfig, PartialIniGetDefaults) {
    IniFile ini;
    ini.LoadFromString("[zoom]\ndefaultZoomLevel = 4.00\n");
    AppConfig c = AppConfigFromIni(ini);

    EXPECT_FLOAT_EQ(c.zoom.defaultZoomLevel, 4.0f);
    // Everything else should be defaults
    EXPECT_EQ(c.draw.freehand.width, 3);
    EXPECT_EQ(c.timer.durationSeconds, 600);
    EXPECT_TRUE(c.toolbar.visible);
}

TEST(AppConfig, UnknownKeysIgnored) {
    IniFile ini;
    ini.LoadFromString("[zoom]\nfutureKey = 42\ndefaultZoomLevel = 2.50\n[unknownSection]\nfoo = bar\n");
    AppConfig c = AppConfigFromIni(ini);
    EXPECT_FLOAT_EQ(c.zoom.defaultZoomLevel, 2.5f);
}

TEST(HotkeyConfig, IniRoundTrip) {
    AppConfig orig;
    orig.zoom.hotkey.modifiers = MOD_CONTROL | MOD_SHIFT;
    orig.zoom.hotkey.vkey = 'A';

    IniFile ini;
    AppConfigToIni(orig, ini);
    AppConfig back = AppConfigFromIni(ini);

    EXPECT_EQ(back.zoom.hotkey.modifiers, MOD_CONTROL | MOD_SHIFT);
    EXPECT_EQ(back.zoom.hotkey.vkey, 'A');
}

// ============================================================================
// IniFile unit tests
// ============================================================================

TEST(IniFile, BasicSetGet) {
    IniFile ini;
    ini.Set("section", "key", "value");
    EXPECT_EQ(ini.Get("section", "key"), "value");
    EXPECT_EQ(ini.Get("section", "missing", "default"), "default");
    EXPECT_EQ(ini.Get("nosection", "key", "default"), "default");
}

TEST(IniFile, IntSetGet) {
    IniFile ini;
    ini.SetInt("s", "k", 42);
    EXPECT_EQ(ini.GetInt("s", "k"), 42);
    EXPECT_EQ(ini.GetInt("s", "missing", 99), 99);
}

TEST(IniFile, FloatSetGet) {
    IniFile ini;
    ini.SetFloat("s", "k", 3.14f);
    EXPECT_NEAR(ini.GetFloat("s", "k"), 3.14f, 0.01f);
    EXPECT_FLOAT_EQ(ini.GetFloat("s", "missing", 1.0f), 1.0f);
}

TEST(IniFile, BoolSetGet) {
    IniFile ini;
    ini.SetBool("s", "on", true);
    ini.SetBool("s", "off", false);
    EXPECT_TRUE(ini.GetBool("s", "on"));
    EXPECT_FALSE(ini.GetBool("s", "off"));
    EXPECT_TRUE(ini.GetBool("s", "missing", true));
}

TEST(IniFile, ParseComments) {
    IniFile ini;
    ini.LoadFromString("[s]\n# comment\n; another comment\nkey = val\n");
    EXPECT_EQ(ini.Get("s", "key"), "val");
}

TEST(IniFile, ParseWhitespace) {
    IniFile ini;
    ini.LoadFromString("[s]\n  key  =  value with spaces  \n");
    EXPECT_EQ(ini.Get("s", "key"), "value with spaces");
}

TEST(IniFile, StringRoundTrip) {
    IniFile ini;
    ini.Set("a", "x", "1");
    ini.Set("a", "y", "2");
    ini.Set("b", "z", "3");

    std::string content = ini.SaveToString();
    IniFile ini2;
    ini2.LoadFromString(content);

    EXPECT_EQ(ini2.Get("a", "x"), "1");
    EXPECT_EQ(ini2.Get("a", "y"), "2");
    EXPECT_EQ(ini2.Get("b", "z"), "3");
}
