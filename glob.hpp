#ifndef CHOC_WILDCARD_HEADER_INCLUDED
#define CHOC_WILDCARD_HEADER_INCLUDED

#include "../text/choc_UTF8.h"

namespace choc::text
{

//==============================================================================
/**
    This handles the kind of simple wildcards that you'd give to a filesystem search.
    It recognises: - '*' = any sequence of zero or more characters
                  - '?' = any character
    It accepts a string containing multiple patterns separated by semi-colons, and
    considers it a successful match if any of these match.
    E.g. "*.foo;*.blah" will match strings that end either either ".foo" or ".blah".
*/
struct glob
{
    glob() = default;
    glob (glob&&) = default;
    glob& operator= (glob&&) = default;

    /// Creates a matcher for the given pattern.
    glob (std::string_view pattern, bool caseSensitive);

    /// Returns true if the given string matches the pattern.
    bool matches (const std::string& text) const;

    /// You can iterate the pattern strings within the wildcard
    std::vector<std::string>::iterator begin()  { return patterns.begin(); }
    std::vector<std::string>::iterator end()    { return patterns.end(); }

private:
    std::vector<std::string> patterns;
    bool isCaseSensitive;

    bool matchesAnywhere (choc::text::UTF8Pointer, choc::text::UTF8Pointer) const;
    bool matchesAll (choc::text::UTF8Pointer, choc::text::UTF8Pointer) const;
};



//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

inline glob::glob (std::string_view pattern, bool caseSensitive)
    : isCaseSensitive (caseSensitive)
{
    for (auto& p : choc::text::splitString (pattern, ';', false))
        patterns.push_back (choc::text::trim (p));
}

inline bool glob::matches (const std::string& text) const
{
    if (patterns.empty())
        return true;

    choc::text::UTF8Pointer t (text.c_str());

    for (auto& p : patterns)
        if (matchesAll (t, choc::text::UTF8Pointer (p.c_str())))
            return true;

    return false;
}

inline bool glob::matchesAnywhere (choc::text::UTF8Pointer source, choc::text::UTF8Pointer pattern) const
{
    while (! source.empty())
    {
        if (matchesAll (source, pattern))
            return true;

        ++source;
    }

    return false;
}

inline bool glob::matchesAll (choc::text::UTF8Pointer source, choc::text::UTF8Pointer pattern) const
{
    for (;;)
    {
        auto patternChar = pattern.popFirstChar();

        if (patternChar == '*')
            return pattern.empty() || matchesAnywhere (source, pattern);

        auto sourceChar = source.popFirstChar();

        if (! (sourceChar == patternChar
                || (! isCaseSensitive && std::towupper (static_cast<std::wint_t> (sourceChar)) == std::towupper (static_cast<std::wint_t> (patternChar)))
                || (patternChar == '?' && sourceChar != 0)))
            return false;

        if (patternChar == 0)
            return true;
    }
}
} // namespace choc::text

#endif // CHOC_WILDCARD_HEADER_INCLUDED
