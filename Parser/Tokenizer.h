#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#ifdef _USRDLL
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT __declspec(dllimport)
#endif

#include <string>
#include <xstring>
#include <array>
#include <map>
#include <list>
#include <vector>
#include <boost/filesystem.hpp>
#include "ReadFileData.h"

using namespace std;

class Tokenizer;

struct EXPORT Token
{
public:
    enum TokenType {
        endOfInput = -1,

        unknown = 0,

        incomplete,
        comment,
        number,
        string,
        assignment,
        scope,


        sectionSize = 1024,

        keyword = 1024,

        identifier = 2048,

        block = 3072,
        block_start,
        block_end,
        params_start,
        params_end,

        failures = 4096,
        badString,
    };

    enum EXPORT ParsingFlags
    {
        none = 0,

        needs = 1,

        // states
        testResult = 2,
        needsTestResult = 4,

        elseAllowed = 8,
        needsElseAllowed = 16,

        loopBlock = 32,
        needsLoopBlock = 64,

        value = 128,
        needsValue = 256,

        // token following order
        needsParameters = 1024,
        needsBlock = 2048,
        needsIdentifier = 4096,
    };

    struct EXPORT TypeAndParsingFlags {
        int type;
        int parsingFlags;

        TypeAndParsingFlags(
            int inType = TokenType::unknown,
            int inFlags = ParsingFlags::none);
    };

    bool            hasScope;
    long            startingLine;
    long            startingCharacter;
    string_view    tokenString;
    TypeAndParsingFlags typeAndFlags;

    void Init(
        long inStartingLine,
        long inStartingCharacter,
        int inType = Token::incomplete,
        int inParsingFlags = ParsingFlags::none);

    Token(
        long inStartingLine = 0,
        long inStartingCharacter = 0,
        int inType = Token::incomplete,
        int inParsingFlags = ParsingFlags::none);;

    bool IsKeyword();
    bool isIdentifier();
    bool isBlock();
};

class EXPORT Tokenizer {
private:
    // The tokenize functions handle setting up the readData
    void internalTokenize(const char* start, const char* end);

public:
    // configuration flags
    bool allowDoubleQuoteStrings;
    bool allowSingleQuoteStrings;
    char scopeChar;


    // Collections
    map<string_view, Token::TypeAndParsingFlags>& keywordTokens;


    list<ReadFileData*> sourceFileData;

    vector<Token>& tokens;

    // Constructor
    Tokenizer(map<string_view, Token::TypeAndParsingFlags>* inKeywordTokens);

    void tokenize(istream& input);
    void tokenize(boost::filesystem::path filePath);
    void tokenize(string_view stringBuffer);

    // Load the token

    bool testIsNumber(string_view& maybeNumber);

    void finishToken(
        Token& token,
        const char*& start,
        const char* end,
        int inType = Token::unknown);

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
private:
    vector<Token>& tokenRecovery;
};

#endif // TOKENIZER_H_INCLUDED

