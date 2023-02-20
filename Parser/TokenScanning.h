#pragma once

// These are all utility founctions for finding typed tokens
struct MatchInfo
{
    size_t  length;
    long    lines;
    long    chars;
    char    id;


    void clear()
    {
        length = 0;
        lines = 0;
        chars = 0;
        id = 0;
    }

    MatchInfo()
    {
        clear();
    }
};

// I wanted to use regex for the tokenMatcher but C++ has regex issues
// so instead here are some character buffer token matchers.
MatchInfo StartEndMatcher(const char* start, const char* end, char startEnd);
MatchInfo HexMatcher(const char* start, const char* end, char special, const bool prefixMatched);
MatchInfo HexMatcher(const char* start, const char* end, char special = 0);
MatchInfo NuberMatcher(const char* start, const char* end, char special = 0);
MatchInfo IdentifierMatcher(const char* start, const char* end, char special = 0);
MatchInfo WhiteSpaceMatcher(const char* start, const char* end, char special = 0);
MatchInfo PunctuationMatch(const char* start, const char* end, char special = 0);
MatchInfo LineCommentMatcher(const char* start, const char* end, char special = '/');
