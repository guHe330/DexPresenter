#pragma once
#include <Windows.h>
#include <gdiplus.h>
#include <string>

// Generate a timestamped screenshot path: dir\prefix_2026-03-05T14-30-45.png
std::wstring GenerateScreenshotPath(const std::wstring& dir, const std::wstring& prefix);

// Save an HBITMAP as PNG using GDI+. Returns true on success.
bool SaveBitmapAsPng(HBITMAP hBmp, const std::wstring& path);

// Get default screenshot directory: %USERPROFILE%\Pictures\DexPresenter
std::wstring GetDefaultScreenshotDir();

// Ensure directory exists (creates recursively). Returns true on success.
bool EnsureDirectoryExists(const std::wstring& path);
