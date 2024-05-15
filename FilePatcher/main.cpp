#include <iostream>
#include "patcher.h"
#include "stringmanip.h"
#include "system.h"

namespace sm = my::stringmanip;
namespace sys = my::system;

int wmain(int argc, wchar_t* argv[])
{
    int returnCode = EXIT_SUCCESS;

    try
    {
        sys::setConsoleUTF8(true);

        if (argc != 6)
        {
            std::string programName = sys::getProgramName(sm::toString(argv[0]));
            std::cerr << "Usage: " << programName << " <input file> <output file> <pattern> <replacement> <occurrence>" << std::endl;
            std::cerr << "- Pattern and replacement should be hexadecimal strings without spaces, e.g., DEADBEEF BAADC0DE." << std::endl;
            std::cerr << "- Use ? for wildcards in the pattern and replacement, e.g., DE??BEEF BAADC0DE, or DE??BEEF BA??C0DE to keep the wildcard content unchanged." << std::endl;
            std::cerr << "- Occurrence should be 'all' or a number starting from 1." << std::endl;
            std::cerr << std::endl;
            std::cerr << "Example : " << programName << " myfile myfile_patched DE??BEEF BA??C0DE 1" << std::endl;
            return EXIT_FAILURE;
        }

        std::string inputFilename = sm::toString(argv[1]);
        std::string outputFilename = sm::toString(argv[2]);
        std::string patternHex = sm::toString(argv[3]);
        std::string replacementHex = sm::toString(argv[4]);
        std::string occurrence = sm::toString(argv[5]);

        // Convert hex strings to byte vectors
        Pattern pattern = Patcher::hexStringToBytes(patternHex);
        Pattern replacement = Patcher::hexStringToBytes(replacementHex);
        int nOccurence = (occurrence == "all") ? 0 : std::stoul(occurrence);

        // Patch the binary file
        Patcher patcher(inputFilename, outputFilename);
        patcher.replace(pattern, replacement, nOccurence);

        std::cout << "Patching started..." << std::endl << std::endl;
        patcher.patch();
        std::cout << "Patching completed successfully!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        returnCode = EXIT_FAILURE;
    }

    // Make sure to set the console encoding back even if there was an error
    try
    {
        sys::setConsoleUTF8(false);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        returnCode = EXIT_FAILURE;
    }

    return returnCode;
}