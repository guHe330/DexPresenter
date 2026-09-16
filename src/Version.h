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

// The single source of truth for the version. CMakeLists.txt parses this file,
// DexPresenter.rc includes it, and the release workflow refuses to publish a tag
// that disagrees with it -- so this is the only place a version is ever bumped.
#define DEXPRESENTER_VERSION        L"1.0.0"
#define DEXPRESENTER_VERSION_MAJOR  1
#define DEXPRESENTER_VERSION_MINOR  0
#define DEXPRESENTER_VERSION_PATCH  0

// VERSIONINFO wants a comma list and a narrow string; both are the same version
// with a trailing build field of 0, which DexPresenter does not use.
#define DEXPRESENTER_VERSION_COMMA  1,0,0,0
#define DEXPRESENTER_VERSION_DOTTED "1.0.0.0"
