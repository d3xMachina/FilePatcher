#pragma once
#include <string>

namespace my::stringmanip
{
	std::wstring toWString(const std::string& str);
	std::string toString(const std::wstring& wstr, unsigned int codePage);
	std::string toString(const std::wstring& wstr);
	std::string toString(const std::u8string& u8Str);
	void removeTrailingNull(std::string& str);
	void removeTrailingNull(std::wstring& str);
	bool isValidHexChar(char c);
	bool isPositiveInteger(const std::string &str);
}
