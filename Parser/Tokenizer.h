#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include "pch.h"
#include "interop.h"
#include "TokenScanning.h"
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
    static map<long, string_view> tokenTypeNames;

public:
    enum TokenType
    {
        nonToken,
        notAFailure,
        endOfInput,

        unknownToken,

        incomplete,
        comment,
        multiLineComment,
        identifier,
        number,
        hexNumber,
        stringValue,
        assignment,
        scope,
        member,
        name,
        keyword,
        functionReturn,
        selfCall,
        compilerFlag,


        sectionSize = 64,

        packageName = 64,

        block = 128,
        block_start,
        block_end,
        params_start,
        params_end,
        prototype_start,
        prototype_end,

        failures = 256,
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
        compileFlag = 1048576,       // For use in contidional compilation.  Most should come from the build environment, but they can be set or unset in modules.
    };


    bool            hasScope;
    long            startingLine;
    long            startingCharacter;
    string_view     tokenString;
    long            typeFlags;

    void init(
        long inStartingLine,
        long inStartingCharacter,
        int inType = Token::incomplete);

    Token(
        long inStartingLine = 0,
        long inStartingCharacter = 0,
        int inType = Token::incomplete);

    inline std::string errorDisplay()
    {
        return std::format("\"{:20}\"\nLine: {} Offset: {}", tokenString, startingLine, startingCharacter);
    }

    ostream& OutputTypeFlag(ostream& output);

    bool isKeyword();
    bool isIdentifier();
    bool isBlock();
};

void TokenFlagToString(string& toAppendFlags, long tokenTypeFlags);

class EXPORT TokenMatching
{
public:
    // To make your own matcher your can use lambdas ->
    // [](const char* start, const char* end, char special) -> MatchInfo {...}

    char matchOrSpecial;
    // (*tokenMatcher)(const char* start, const char* end, char special);
    MatchInfo (*tokenMatcher)(const char* start, const char* end, char special);

    ~TokenMatching()
    {
    }

    TokenMatching(char match)
    {
        matchOrSpecial = match;
        tokenMatcher = NULL;
    }

    TokenMatching(
        MatchInfo (*inTokenMatcher)(const char*, const char*, char),
        char special = 0)
    {
        matchOrSpecial = special;
        tokenMatcher = inTokenMatcher;
    }

    bool isSingleCharacterMatch() { return NULL == tokenMatcher; }
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
    void (*idToTokenType)(const MatchInfo& info, Token& token);

    // Collections
    map<char, TokenMatching*>& tokenReadingMap;

    token_vector& tokens;

    // Constructor
    Tokenizer(
        map<char, TokenMatching*>* inTokenReadingMap,
        void (*inIdToTokenType)(const MatchInfo&, Token&)
    ) :
        idToTokenType(inIdToTokenType),
        tokenReadingMap(*inTokenReadingMap),
        tokens(*(new token_vector()))
    {}

    void tokenize(istream& input);
    void tokenize(boost::filesystem::path& filePath);
    void tokenize(string_view stringBuffer);

    // Cleanup
    void cleanup();
};

#endif // TOKENIZER_H_INCLUDED

