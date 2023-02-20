#include "pch.h"

using namespace std;

MatchInfo StartEndMatcher(const char* start, const char* end, char startEnd)
{
    MatchInfo result;

    const char* runner = start;

    if (runner < end && startEnd == *runner)
    {
        bool isEscaped = false;
        while (++runner < end)
        {
            if (!isEscaped && startEnd == *runner)
            {
                // +1 include the final quote
                result.length = 1L + (runner - start);
                result.id = startEnd;

                return result;
            }
            else if ('\\' == *runner)
            {
                // \ must toggle - \" = escaped, \\" = \ escaped " not, \\\" \ escaped " escaped  
                isEscaped = !isEscaped;
            }
            else if ('\n' == *runner)
            {
                result.lines++;
                result.chars = 0;   // 0 -> result.chars++ will happen before looping
                isEscaped = false;
            }
            else
            {
                isEscaped = false;
            }
            result.chars++;
        }
    }

    return result;  // Non  match
};

MatchInfo HexMatcher(const char* start, const char* end, char special, const bool prefixMatched)
{
    MatchInfo result;

    const char* pos = start;

    bool isHex = false;

    if (!prefixMatched)
    {
        pos++;
        if ((pos < end) && '0' == *start
            && ('x' == *pos || 'X' == *pos))
        {
            // prefix matched
            pos++;
        }
        else
        {
            // non match
            return result;
        }
    }
    else
    {
        pos += 2;
    }

    while (pos < end && (('0' <= *pos && *pos <= '9') ||
        ('a' <= *pos && *pos <= 'f') ||
        ('A' <= *pos && *pos <= 'F')))
    {
        pos++;
        isHex = true;
    }

    if (isHex && ((pos == end) || ispunct(*pos) || isspace(*pos)))
    {
        // Single line we have not crossed \n
        result.chars = static_cast<long>(result.length = pos - start);
        result.id = 'x'; // 'x' for hex number identifier
    }

    return result;
}

MatchInfo HexMatcher(const char* start, const char* end, char special)
{
    return HexMatcher(start, end, special, false);
}


MatchInfo NuberMatcher(const char* start, const char* end, char special)
{
    // Could be a Hex number
    if ((start + 1 < end) && '0' == *start
        && ('x' == *(start + 1) || 'X' == *(start + 1)))
    {
        return HexMatcher(start, end, 0, true);
    }

    MatchInfo result;

    const char* pos = start;

    bool negativeAllowed = true;
    bool dotAllowed = true;
    bool eAllowed = false;
    bool containsE = false;
    bool valid = false;

    while (pos < end)
    {
        if ('0' <= *pos && *pos <= '9')
        {
            valid = true;
            negativeAllowed = false;

            // Exponent allowed after a digit if it is not already in the number.
            if (!containsE) eAllowed = true;
        }
        else if ('.' == *pos && dotAllowed)
        {
            dotAllowed = false;
            negativeAllowed = false;
        }
        else if (('e' == *pos || 'E' == *pos) && eAllowed)
        {
            // Need a digit after 'e' to be valid again!
            valid = eAllowed = dotAllowed = false;

            // Can have a negative exponent
            containsE = negativeAllowed = true;
        }
        else if ('-' == *pos && negativeAllowed)
        {
            negativeAllowed = false;
        }
        else break;

        pos++;
    }

    if ((valid && ((pos == end) || isspace(*pos)
        || (ispunct(*pos) && '-' != *pos && '.' != *pos))))
    {
        // Single line we have not crossed \n
        result.chars = static_cast<long>(result.length = pos - start);
        if (0 < result.length) result.id = '1';  // '1' decimal number identifier
    }
    return result;
};

MatchInfo IdentifierMatcher(const char* start, const char* end, char special)
{
    MatchInfo result;

    const char* pos = start;

    // (isprint(*pos) || (0x80 & *pos)) - used to allow all multi-byte utf8
    // This means some utf8 punctuation can be used in identifiers, but that is OK

    bool oneTimeMatch;
    if (pos < end && (isprint(*pos) || (0x80 & *pos)) &&
        ((oneTimeMatch = 0 != special && special == *pos) ||
            ('_' == *pos || (!ispunct(*pos) && !isspace(*pos) && !isdigit(*pos)))))
    {
        pos++;

        if (oneTimeMatch)
        {
            result.id = special;
            special = 0;
        }

        // Digits allowed now.
        while (pos < end && (isprint(*pos) || (0x80 & *pos)) &&
            ((oneTimeMatch = 0 != special && special == *pos) ||
                ('_' == *pos || (!ispunct(*pos) && !isspace(*pos)))))
        {
            pos++;

            if (oneTimeMatch)
            {
                result.id = special;   // Use the special as the special identifier
                special = 0;
            }
        }

        // Single line we have not crossed \n
        result.chars = static_cast<long>(result.length = pos - start);
        if (0 < result.length && 0 == result.id) result.id = 'a'; //First letter for generic identifier.
    }

    return result;
}

MatchInfo WhiteSpaceMatcher(const char* start, const char* end, char special)
{
    MatchInfo result;

    const char* pos = start;

    while (pos < end && isspace(*pos))
    {
        if ('\n' == *pos)
        {
            result.lines++;
            result.chars = 0;   // 0 -> result.chars++ will happen before looping
        }
        pos++;
        result.chars++;
    }

    result.length = pos - start;
    if (0 < result.length) result.id = ' ';  // ' ' whitespace identifier

    return result;
}

MatchInfo PunctuationMatch(const char* start, const char* end, char special)
{
    MatchInfo result;

    const char* pos = start;

    if (ispunct(*start))
    {
        result.length = 1;
        result.id = '|';  // '|' punctuation identifier
        result.chars = 1;
    }

    return result;
}

MatchInfo LineCommentMatcher(const char* start, const char* end, char special)
{
    MatchInfo result;

    const char* pos = start;

    if (pos < end && special == *pos)
    {
        while (pos < end && '\n' != *pos) pos++;

        result.length = pos - start;
        result.id = special;  // special - the comment characer
        result.lines = 1;
        result.chars = 1;
    }

    return result;
}
