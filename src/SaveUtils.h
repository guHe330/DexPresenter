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
