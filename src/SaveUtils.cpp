#include "SaveUtils.h"
#include <ShlObj.h>
#include <filesystem>
#include <chrono>

// Get the CLSID of the PNG encoder
static bool GetPngEncoderClsid(CLSID* clsid) {
    UINT num = 0, size = 0;
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return false;

    auto buffer = std::make_unique<BYTE[]>(size);
    auto encoders = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.get());
    Gdiplus::GetImageEncoders(num, size, encoders);

    for (UINT i = 0; i < num; i++) {
        if (wcscmp(encoders[i].MimeType, L"image/png") == 0) {
            *clsid = encoders[i].Clsid;
            return true;
        }
    }
    return false;
}

std::wstring GetDefaultScreenshotDir() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_MYPICTURES, nullptr, 0, path))) {
        return std::wstring(path) + L"\\DexPresenter";
    }
    // Fallback to %USERPROFILE%\Pictures\DexPresenter
    wchar_t* profile = nullptr;
    _wdupenv_s(&profile, nullptr, L"USERPROFILE");
    std::wstring dir;
    if (profile) {
        dir = std::wstring(profile) + L"\\Pictures\\DexPresenter";
        free(profile);
    } else {
        dir = L"C:\\DexPresenter";
    }
    return dir;
}

bool EnsureDirectoryExists(const std::wstring& path) {
    try {
        std::filesystem::create_directories(path);
        return true;
    } catch (...) {
        return false;
    }
}

std::wstring GenerateScreenshotPath(const std::wstring& dir, const std::wstring& prefix) {
    EnsureDirectoryExists(dir);

    // Get current local time
    SYSTEMTIME st;
    GetLocalTime(&st);

    wchar_t filename[256];
    swprintf_s(filename, L"%s_%04d-%02d-%02dT%02d-%02d-%02d.png",
        prefix.c_str(),
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);

    return dir + L"\\" + filename;
}

bool SaveBitmapAsPng(HBITMAP hBmp, const std::wstring& path) {
    CLSID clsid;
    if (!GetPngEncoderClsid(&clsid)) return false;

    Gdiplus::Bitmap bitmap(hBmp, nullptr);
    if (bitmap.GetLastStatus() != Gdiplus::Ok) return false;

    return bitmap.Save(path.c_str(), &clsid) == Gdiplus::Ok;
}
