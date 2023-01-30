#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include "pch.h"
#include "interop.h"
#include <string>
#include <xstring>
#include <array>
#include <map>
#include <list>
#include <format>
#include <regex>
#include <boost/filesystem.hpp>
#include "ReadFileData.h"

using namespace std;

class Tokenizer;

struct EXPORT Token
{
public:
    enum TokenType
    {
        notAFailure = -2,
        endOfInput = -1,

        unknownToken = 0,

        incomplete,
        comment,
        number,
        hexNumber,
        stringValue,
        assignment,
        scope,
        member,
        name,
        functionReturn,
        selfCall,
        compilerFlag,


        sectionSize = 1024,

        keyword = 1024,

        identifier = 2048,

        block = 3072,
        block_start,
        block_end,
        params_start,
        params_end,
        prototype_start,
        prototype_end,

        failures = 4096,
        badString,
        badNumber,
        badPunctuation,
        badUnknown
    };

    enum ParsingFlags
    {
        none = 0,

        // states
        loopBlock = 1,

        allowedInParameters = 2,

        value = 4,

        functionDefinition = 8,

        testResult = 16,
        elseAllowed = 32,


        _requirementsFlags = 64,
        requiresTestResult = 64,
        requiresElseAllowed = 128,
        requiresLoopBlock = 256,
        requiresFunctionBlock = 512,
        _requirementsFlagsMax = 1024 - 1,


        _followsFlags = 16384,        // 2^14
        // token following order
        valueFollows = 16384,
        parametersFollows = 32768,
        identifierFollows = 65536,
        nameFollows = 131072,
        compileFlagFollows = 262144,

        blockFollows = 524288,        // 2^19
        _followsFlagsMax = 1048576 - 1,

        _extensionFlags = 1048576,   // 2^20
        compileFlag = 1048576,         // For use in contidional compilation.  Most should come from the build environment, but they can be set or unset in modules.
    };


    struct EXPORT TypeAndParsingFlags {
        long type;
        long parsingFlags;

        inline TypeAndParsingFlags(
            long inType = Token::unknownToken,
            long inFlags = Token::none)
        {
            type = inType;
            parsingFlags = inFlags;
        }

        inline bool hasParsingFlags(long flagsToCheck)
        {
            return flagsToCheck == (parsingFlags & flagsToCheck);
        }

        inline bool hasRequirementsFlags()
        {
            return  Token::_requirementsFlags <= (parsingFlags & Token::_requirementsFlagsMax);
        }

        inline bool hasFollowsFlags()
        {
            return  Token::_followsFlags <= (parsingFlags & Token::_followsFlagsMax);
        }
    };

    bool            hasScope;
    long            startingLine;
    long            startingCharacter;
    string_view     tokenString;
    TypeAndParsingFlags typeAndFlags;

    void init(
        long inStartingLine,
        long inStartingCharacter,
        int inType = Token::incomplete,
        int inParsingFlags = Token::none);

    Token(
        long inStartingLine = 0,
        long inStartingCharacter = 0,
        int inType = Token::incomplete,
        int inParsingFlags = Token::none);

    long checkHexNumberToken();
    long checkNumberToken();

    inline bool hasParsingFlags(long flagsToCheck)
    {
        return typeAndFlags.hasParsingFlags(flagsToCheck);
    }
    inline bool hasRequirementsFlags()
    {
        return  typeAndFlags.hasRequirementsFlags();
    }

    inline bool hasFollowsFlags()
    {
        return  typeAndFlags.hasFollowsFlags();
    }

    inline std::string errorDisplay()
    {
        return std::format("\"{:20}\"\nLine: {} Offset: {}", tokenString, startingLine, startingCharacter);
    }

    bool isKeyword();
    bool isIdentifier();
    bool isBlock();
};

class EXPORT TokenMatching
{
public:
    // I wanted to use regex for the tokenMatcher
    // but C++ has regex issues so instead here are some
    // token matchers
    static long StringMatcher(const char* start, const char* end);
    static long HexMatcher(const char* start, const char* end);
    static long DecimalMatcher(const char* start, const char* end);
    static long IdentifierMatcher(const char* start, const char* end);
    static long RestOfLineMatcher(const char* start, const char* end);
    // To make your own matcher your can use lambdas ->
    // [](const char* start, const char* end) -> long {...}

    // (*tokenMatcher)(start, end);
    long (*tokenMatcher)(const char*, const char*);
    Token::TokenType   success;
    Token::TokenType   failure;
    TokenMatching*     nextPossibleMatch;
    bool               mayBeMultiline;

    ~TokenMatching()
    {
        delete nextPossibleMatch;
        nextPossibleMatch = NULL;
    }

    TokenMatching(Token::TokenType inSuccess)
    {
        tokenMatcher = NULL;
        success = inSuccess;
        failure = Token::badUnknown;
        nextPossibleMatch = NULL;
        mayBeMultiline = false;
    }

    TokenMatching(
        long (*inMatcher)(const char*, const char*),
        Token::TokenType inSuccess,
        Token::TokenType inFailure,
        bool inMayBeMultiline = false)
    {
        tokenMatcher = inMatcher;
        success = inSuccess;
        failure = inFailure;
        nextPossibleMatch = NULL;
        mayBeMultiline = inMayBeMultiline;
    }

    TokenMatching(
        TokenMatching* next,
        long (*inMatcher)(const char* start, const char* end),
        Token::TokenType inSuccess,
        bool inMayBeMultiline = false)
    {
        tokenMatcher = inMatcher;
        success = inSuccess;
        failure = Token::TokenType::notAFailure;
        nextPossibleMatch = next;
        mayBeMultiline = inMayBeMultiline;
    }

    bool isSingleCharacterMatch() { return NULL == tokenMatcher; }

    void findNextToken(Token& nextToken, long& characterNumber, long& lineNumber, const char*& runner, const char* end);

protected:
    long countCharactersAndLineBreaks(const char*& runner, const char* end, long& characterNumber);
};

// token_vector basically just exists to add the EXPORT. IT is not just to simplify using token_vector
class EXPORT token_vector : public std::vector<Token>
{
public:
    token_vector();
};

class EXPORT Tokenizer {
protected:
    // Find all the tokens
    void internalTokenize(const char*& runner, const char* end);

    list<ReadFileData*> sourceFileData;

public:
    // Collections
    map<char, TokenMatching*>& tokenReadingMap;


    token_vector& tokens;

    // Constructor
    Tokenizer(map<char, TokenMatching*>* inTokenReadingMap);

    void tokenize(istream& input);
    void tokenize(boost::filesystem::path& filePath);
    void tokenize(string_view stringBuffer);

    // Cleanup
    void cleanup();
};

#endif // TOKENIZER_H_INCLUDED

