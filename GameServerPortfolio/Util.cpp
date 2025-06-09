#include "stdafx.h"
#include "Util.h"
#include <codecvt>

std::wstring Utf8ToWString(const std::string& utf8) 
{
    if (utf8.empty()) return L"";

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wstr[0], size_needed);

    // Remove null terminator at end
    if (!wstr.empty() && wstr.back() == L'\0') wstr.pop_back();
    return wstr;
}

std::string WStringToUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) return "";

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, nullptr, nullptr);

    // Remove null terminator at end
    if (!str.empty() && str.back() == '\0') str.pop_back();
    return str;
}