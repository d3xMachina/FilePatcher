#include "patcher.h"
#include "stringmanip.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

#define WILDCARD '?'

namespace sm = my::stringmanip;

Patch::Patch(const Pattern& lookup, const Pattern& replacement, size_t occurrence)
    : lookup(lookup), replacement(replacement), occurrence(occurrence), occurrenceCounter(0) { }

Patcher::Patcher(const std::string& inputFilename, const std::string& outputFilename)
    : m_inputFilename(inputFilename), m_outputFilename(outputFilename) { }

// Convert a string of hex digits to a vector of bytes
Pattern Patcher::hexStringToBytes(const std::string& hex)
{
    if (hex.size() % 2 != 0)
    {
        throw std::runtime_error("Hex string length must be even");
    }

    Pattern bytes;
    for (size_t i = 0; i < hex.length(); i += 2)
    {
        if (!sm::isValidHexChar(hex[i]) || !sm::isValidHexChar(hex[i + 1]))
        {
            throw std::runtime_error("Invalid character in hex string");
        }

        char highNibble = hex[i];
        char lowNibble = hex[i + 1];
        int byteValue = 0;

        if (highNibble == WILDCARD)
        {
            byteValue |= PatternFlag::WildcardHigh;
        }
        else
        {
            byteValue |= std::stoi(std::string(1, highNibble), nullptr, 16) << 4;
        }

        if (lowNibble == WILDCARD)
        {
            byteValue |= PatternFlag::WildcardLow;
        }
        else
        {
            byteValue |= std::stoi(std::string(1, lowNibble), nullptr, 16);
        }

        bytes.push_back(byteValue);
    }
    return bytes;
}

void Patcher::replace(const Pattern& pattern, const Pattern& replacement, int occurrence)
{
    if (pattern.size() != replacement.size())
    {
        throw std::runtime_error("Pattern and replacement must be of the same size");
    }

    Patch patch(pattern, replacement, occurrence);
    m_patches.push_back(patch);
}

size_t Patcher::getBiggestPatternSize() const
{
    auto sizeComparer = [](const Patch& a, const Patch& b)
    {
        return a.lookup.size() < b.lookup.size();
    };

    auto maxPatchIt = std::max_element(m_patches.begin(), m_patches.end(), sizeComparer);
    size_t maxPatternSize = (maxPatchIt != m_patches.end()) ? maxPatchIt->lookup.size() : 0;

    return maxPatternSize;
}

// Read and patch the file in chunks
void Patcher::patch()
{
    std::ifstream inputFile(m_inputFilename, std::ios::binary);
    std::ofstream outputFile(m_outputFilename, std::ios::binary);

    if (!inputFile)
    {
        throw std::runtime_error("Failed to open input file: " + m_inputFilename);
    }

    if (!outputFile)
    {
        throw std::runtime_error("Failed to open output file: " + m_outputFilename);
    }

    size_t biggestPatternSize = getBiggestPatternSize();
    size_t overlapMaxSize = biggestPatternSize > 0 ? biggestPatternSize - 1 : 0; // -1 because there is minimum 1 byte that is overlapping at the end of the buffer
    const size_t bufferSize = std::max(defaultBufferSize, biggestPatternSize);

    std::vector<unsigned char> buffer(bufferSize + overlapMaxSize); // Extend buffer to handle overlap
    size_t bufferOffset = 0;
    size_t offset = 0;

    auto patternComparer = [](const unsigned char byte, const Byte patternByte)
    {
        unsigned char mask = 0xFF;
        if (patternByte & PatternFlag::WildcardHigh) mask &= 0x0F;
        if (patternByte & PatternFlag::WildcardLow)  mask &= 0xF0;
        return (byte & mask) == (static_cast<unsigned char>(patternByte) & mask);
    };

    while (inputFile)
    {
        inputFile.read(reinterpret_cast<char*>(buffer.data() + bufferOffset), bufferSize);
        std::streamsize bytesRead = inputFile.gcount();
        size_t bufferContentSize = bytesRead + bufferOffset;

        // Check for end of file
        if (bytesRead == 0)
        {
            outputFile.write(reinterpret_cast<const char*>(buffer.data()), bufferContentSize);
            break;
        }

        // Search and replace pattern in the buffer
        auto it = buffer.begin();
        auto afterMatchIt = it;
        std::vector<unsigned char>::iterator itContentEnd;
        while (true)
        {
            itContentEnd = buffer.begin() + bufferContentSize;
            auto match = multiSearch(it, itContentEnd, m_patches.begin(), m_patches.end(), patternComparer);
            it = match.first;
            if (it == itContentEnd)
            {
                break;
            }

            auto patch = *(match.second);

            std::cout << "Patching " << bytesToHexString(it, it + patch.replacement.size()) << std::endl;                          

            // Replace the bytes without changing the ones in wildcard
            for (size_t i = 0; i < patch.replacement.size(); ++i)
            {
                Byte replaceByte = patch.replacement[i];
                unsigned char& patchedByte = *(it + i);
                unsigned char mask = 0xFF;

                if (replaceByte & PatternFlag::WildcardHigh) mask &= 0x0F;
                if (replaceByte & PatternFlag::WildcardLow)  mask &= 0xF0;

                patchedByte = static_cast<unsigned char>((replaceByte & mask) | (patchedByte & ~mask));
            }

            std::cout << "    with " << bytesToHexString(it, it + patch.replacement.size()) << std::endl
                      << "    at offset 0x" << std::hex << std::uppercase << offset + std::distance(buffer.begin(), it) << std::endl
                      << std::endl;

            it += patch.replacement.size(); // move past the replaced pattern
            afterMatchIt = it;

            // Optimization: remove the patch from the list to avoid checking for it next passes
            if (patch.occurrence != 0 && patch.occurrence == patch.occurrenceCounter)
            {
                m_patches.erase(match.second);
                biggestPatternSize = getBiggestPatternSize();
            }
        }

        // Get the size of the remaining non patched data in the overlap part to add it in the next pass
        size_t overlapSize = overlapMaxSize;
        if (afterMatchIt != buffer.begin())
        {
            overlapSize = distanceCapped(afterMatchIt, itContentEnd, overlapMaxSize);
            overlapMaxSize = biggestPatternSize > 0 ? biggestPatternSize - 1 : 0; // Optimization: reduce overlap size
        }

        size_t bytesPatched = bufferContentSize - overlapSize;
        bufferOffset = overlapSize;
        offset += bufferContentSize - overlapSize;

        // Write patched data to output file, excluding the overlap portion
        outputFile.write(reinterpret_cast<const char*>(buffer.data()), bytesPatched);

        if (overlapSize > 0)
        {
            // Shift the overlap portion to the beginning of the buffer
            auto overlapIt = buffer.begin() + bytesPatched;
            std::copy(overlapIt, overlapIt + overlapSize, buffer.begin());
        }
    }
}