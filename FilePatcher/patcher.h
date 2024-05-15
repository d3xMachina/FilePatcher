#pragma once
#include <algorithm>
#include <iomanip>
#include <list>
#include <sstream>
#include <string>
#include <vector>

typedef int Byte;
typedef std::vector<Byte> Pattern;

enum PatternFlag
{
	WildcardHigh = 1 << 31,
	WildcardLow = 1 << 30
};

struct Patch
{
    Patch(const Pattern& lookup, const Pattern& replacement, size_t occurrence);
    Pattern lookup;
    Pattern replacement;
    size_t occurrence;
    mutable size_t occurrenceCounter;
};

class Patcher
{
public:
    static Pattern hexStringToBytes(const std::string& hex);

	Patcher(const std::string& inputFilename, const std::string& outputFilename);
	void replace(const Pattern& lookup, const Pattern& replacement, int occurrence);
    int patch();
    int notFoundCount() const;

private:
    const size_t defaultBufferSize = 4096;

    size_t getBiggestPatternSize() const;

    std::string m_inputFilename;
    std::string m_outputFilename;
    std::list<Patch> m_patches;

    // Templates
	template<typename Iterator>
    static std::string bytesToHexString(Iterator begin, Iterator end)
    {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setfill('0');
        for (auto it = begin; it != end; ++it)
        {
            oss << std::setw(2) << static_cast<int>(*it);
        }
        return oss.str();
    }

    // Check if two ranges match
    template<typename Iterator1, typename Iterator2, typename BinaryPredicate>
    bool matches(Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2, BinaryPredicate pred)
    {
        while (first2 != last2)
        {
            if (first1 == last1 || !pred(*first1, *first2))
            {
                return false;
            }
            ++first1;
            ++first2;
        }
        return true;
    }

    // Search multiple ranges in the given range
    template<typename Iterator, typename SearchIterator, typename BinaryPredicate>
    std::pair<Iterator, SearchIterator> multiSearch(Iterator first, Iterator last, SearchIterator searchFirst, SearchIterator searchLast, BinaryPredicate pred)
    {
        auto bestMatchData = last;
        auto bestMatchSearch = searchLast;

        if (searchFirst != searchLast)
        {
            bool found = false;

            for (auto it = first; it != last; ++it)
            {
                for (auto searchIt = searchFirst; searchIt != searchLast; ++searchIt)
                {
                    const auto& search = *searchIt;
                    const auto& range = search.lookup;

                    // When patching all occurrences, no need to count them. Same when specific occurrence already found.
                    if ((found && search.occurrence == 0) || (search.occurrence == search.occurrenceCounter))
                    {
                        continue;
                    }

                    if (matches(it, last, range.begin(), range.end(), pred))
                    {
                        ++search.occurrenceCounter;

                        if (!found && (search.occurrence == 0 || search.occurrence == search.occurrenceCounter))
                        {
                            bestMatchData = it;
                            bestMatchSearch = searchIt;
                            found = true;
                        }
                    }
                }
            }
        }
        
        return { bestMatchData, bestMatchSearch };
    }

    template <typename Iterator>
    size_t distanceCapped(Iterator first, Iterator last, size_t maxDistance)
    {
        size_t count = 0;
        for (auto it = first; count < maxDistance && it != last; ++count, ++it);
        return count;
    }
};
