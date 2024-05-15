#pragma once
#include <string>


namespace my::system
{
	void setConsoleUTF8(bool enable);
	std::string getProgramName(const std::string& path);
	void createParentDirectories(std::string& filePath);
}