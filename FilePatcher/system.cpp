#include "system.h"
#include <stdexcept>
#include <windows.h>


namespace my::system
{
void setConsoleUTF8(bool enable)
{
    static UINT consoleCP = GetConsoleOutputCP();

    if (enable)
    {
        if (!SetConsoleOutputCP(CP_UTF8))
        {
            throw std::runtime_error("Could not change the console codepage to UTF8");
        }
    }
    else
    {
        if (!SetConsoleOutputCP(consoleCP))
        {
            throw std::runtime_error("Could not revert the console codepage");
        }
    }
}

std::string getProgramName(const std::string& path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos)
    {
        return path; // No directory separator found, return the whole path
    }
    return path.substr(pos + 1); // Extract the filename
}
}