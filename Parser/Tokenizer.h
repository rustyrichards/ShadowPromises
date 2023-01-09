#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include "pch.h"
#include "interop.h"
#include <string>
#include <xstring>
#include <array>
#include <map>
#include <list>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "ReadFileData.h"

using namespace std;

class Tokenizer;

struct EXPORT Token
{
public:
    enum TokenType {
        endOfInput = -1,

        unknownToken = 0,

        incomplete,
        comment,
        number,
        hexNumber,
        string,
        assignment,
        scope,
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
        badPunctuation
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
            long inType = TokenType::unknownToken,
            long inFlags = ParsingFlags::none)
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
        int inParsingFlags = ParsingFlags::none);

    Token(
        long inStartingLine = 0,
        long inStartingCharacter = 0,
        int inType = Token::incomplete,
        int inParsingFlags = ParsingFlags::none);

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
        auto fmt = boost::format("\"%|0.20|\"\nLine: %|d| Offset: %|d|") % tokenString % startingLine % startingCharacter;
        return fmt.str();
    }

    bool isKeyword();
    bool isIdentifier();
    bool isBlock();
};

// token_vector basically just exists to add the EXPORT. IT is not just to simplify using token_vector
class EXPORT token_vector : public std::vector<Token>
{
public:
    token_vector();
};

class EXPORT Tokenizer {
protected:
    // The tokenize functions handle setting up the readData
    void internalTokenize(const char* start, const char* end);

    list<ReadFileData*> sourceFileData;

public:
    // configuration flags
    bool allowDoubleQuoteStrings;
    bool allowSingleQuoteStrings;
    char scopeChar;


    // Collections
    map<string_view, Token::TypeAndParsingFlags>& keywordTokens;


    token_vector& tokens;

    // Constructor
    Tokenizer(map<string_view, Token::TypeAndParsingFlags>* inKeywordTokens);

    void tokenize(istream& input);
    void tokenize(boost::filesystem::path& filePath);
    void tokenize(string_view stringBuffer);

    // Load the token

    void finishToken(
        Token& token,
        const char*& start,
        const char* end,
        int inType = Token::unknownToken);

    void readInComment(
        Token& token,
        const char*& runner,
        const char* end,
        long& characterCount);

    bool readInString(
        char stringStart,
        const char*& runner,
        const char* end,
        long& lineCount,
        long& characterCount,
        bool multilineString = false);

    // handleCurrentCharacterOverride must increment runner if it advances!
    virtual bool handleCurrentCharacterOverride(
        Token& token,
        const char*& start,
        const char*& runner,
        const char* end,
        long& lineCount,
        long& characterCount);

    void handleCurrentCharacter(
        Token& token,
        const char*& start,
        const char*& runner,
        const char* end,
        long& lineCount,
        long& characterCount);

    const char* findNextToken(
        Token& token,
        const char* start,
        const char* end,
        long& lineCount,
        long& characterCount);


    // Cleanup
    void cleanup();
};

#endif // TOKENIZER_H_INCLUDED

