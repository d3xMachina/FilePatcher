#include "stringmanip.h"
#include <string_view>
#include <Windows.h>


namespace my::stringmanip
{
std::wstring toWString(const std::string& str)
{
    std::wstring wstrTo;
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (DWORD) str.size(), NULL, 0);
    if (size > 0)
    {
        wstrTo.resize(size);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (DWORD) str.size(), &wstrTo[0], size);
    }
    return wstrTo;
}

std::string toString(const std::wstring& wstr, unsigned int codePage)
{
    std::string strTo;
    BOOL badCharacter = FALSE;
    int size = WideCharToMultiByte(codePage, WC_NO_BEST_FIT_CHARS, wstr.c_str(), (DWORD) wstr.size(), NULL, 0, NULL, &badCharacter);
    if (!badCharacter && size > 0)
    {
        strTo.resize(size);
        WideCharToMultiByte(codePage, WC_NO_BEST_FIT_CHARS, wstr.c_str(), (DWORD) wstr.size(), &strTo[0], size, NULL, NULL);
    }
    return strTo;
}

std::string toString(const std::wstring& wstr)
{
    return toString(wstr, CP_UTF8);
}

std::string toString(const std::u8string& u8Str)
{
    return std::string(std::string_view(reinterpret_cast<const char*>(u8Str.c_str()), u8Str.size()));
}

void removeTrailingNull(std::string& str)
{
    str = str.c_str();
    str.shrink_to_fit();
}

void removeTrailingNull(std::wstring& str)
{
    str = str.c_str();
    str.shrink_to_fit();
}

bool isValidHexChar(char c)
{
    return std::isxdigit(c) || c == '?';
}
}