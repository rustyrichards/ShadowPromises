#include "pch.h"

#include <iostream>
#include <fstream>      // ifstream

bool Token::isKeyword()
{
    return keyword <= typeFlags && typeFlags < keyword + sectionSize;
}

bool Token::isIdentifier()
{
    return identifier <= typeFlags && typeFlags < identifier + sectionSize;
}

bool Token::isBlock()
{
    return block <= typeFlags && typeFlags < block + sectionSize;
}

void Token::init(
    long inStartingLine, 
    long inStartingCharacter, 
    int inType)
{
    hasScope = false;
    startingLine = inStartingLine;
    startingCharacter = inStartingCharacter;
    tokenString = ""sv;
    typeFlags = inType;
}

Token::Token(
    long inStartingLine,
    long inStartingCharacter,
    int inType) :
    hasScope(false),
    startingLine(inStartingLine),
    startingCharacter(inStartingCharacter),
    tokenString(""sv),
    typeFlags(inType)
{}


token_vector::token_vector() : vector<Token>()
{}

long TokenMatching::countCharactersAndLineBreaks(const char*& runner, const char* end, long& characterNumber)
{
    long lineBreaks = 0;
    while (runner < end)
    {
        characterNumber++;
        if ('\n' == *(runner++))
        {
            lineBreaks++;
            characterNumber = 1;
        }
    }

    return lineBreaks;
}

void failTokenToNextWhitespace(Token& token, Token::TokenType faiureCode, long& characterNumber, long lineNumber, const char*& runner, const char* end)
{
    // Match failure
    token.startingLine = lineNumber;
    token.startingCharacter = characterNumber;
    token.typeFlags = faiureCode;

    // Advance to the next whitespace. And declare the current "word" an error.
    const char* start = runner;
    while (runner != end && !isspace(*runner))
    {
        runner++;
        characterNumber++;
    }
    token.tokenString = string_view(start, runner - start);
}

void TokenMatching::findNextToken(Token& token, long& characterNumber, long& lineNumber, const char*& runner, const char* end)
{
    token.startingLine = lineNumber;
    token.startingCharacter = characterNumber;

    long matchedLen = 0;

    if (isSingleCharacterMatch())
    {
        token.typeFlags = success;
        token.tokenString = string_view(runner++, 1);
        characterNumber++;
    }
    else if ((NULL != tokenMatcher) &&
        (0 < (matchedLen = (*tokenMatcher)(runner, end))))
    {
        token.typeFlags = success;
        token.tokenString = string_view(runner, matchedLen);
        if (mayBeMultiline)
        {
            lineNumber += countCharactersAndLineBreaks(runner, runner + matchedLen, characterNumber);
        }
        else
        {
            characterNumber += matchedLen;
        }
        runner += matchedLen;
    }
    else if (NULL != nextPossibleMatch)
    {
        nextPossibleMatch->findNextToken(token, characterNumber, lineNumber, runner, end);
    }
    else
    {
        failTokenToNextWhitespace(token, failure, characterNumber, lineNumber, runner, end);
    }
}

long TokenMatching::StringMatcher(const char* start, const char* end)
{
    const char* runner = start;
    char stringQuote = *runner;

    if ('"' == stringQuote || '\'' == stringQuote)
    {
        bool isEscaped = false;
        while (++runner < end)
        {
            if (!isEscaped && stringQuote == *runner)
            {
                // +1 include the final quote
                return 1L + (runner - start);
            }
            if ('\\' == *runner)
            {
                // \ must toggle - \" = escaped, \\" = \ escaped " not, \\\" \ escaped " escaped  
                isEscaped = !isEscaped;
            }
            else isEscaped = false;
        }
    }
    return 0;
};

long TokenMatching::HexMatcher(const char* start, const char* end)
{
    const char* pos = start;

    // Need atleast 0xh -> 3 characters
    bool isHex = false;
    if (3 <= end - start)
    {
        pos++;
        if ('0' == *start && ('x' == *pos || 'X' == *pos))
        {
            pos++;
            while (pos < end && (('0' <= *pos && *pos <= '9') ||
                ('a' <= *pos && *pos <= 'f') ||
                ('A' <= *pos && *pos <= 'F')))
            {
                pos++;
                isHex = true;
            }
        }
    }

    return (isHex && ((pos == end) || ispunct(*pos) || isspace(*pos)))
        ? pos - start : 0;
};

long TokenMatching::DecimalMatcher(const char* start, const char* end)
{
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

    return (valid && ((pos == end) || isspace(*pos)
        || (ispunct(*pos) && '-' != *pos && '.' != *pos)))
        ? pos - start : 0;
};

long TokenMatching::IdentifierMatcher(const char* start, const char* end)
{
    const char* pos = start;

    // one colon is allowed for the package
    char allowedColon = ':';

    // (isprint(*pos) || (0x80 & *pos)) - used to allow all multi-byte utf8
    // This means some utf8 punctuation can be used in identifiers, but that is OK
    if (pos < end && (isprint(*pos) || (0x80 & *pos)) && (!ispunct(*pos) || allowedColon == *pos) && !isspace(*pos) && !isdigit(*pos))
    {
        if (':' == *pos) allowedColon = ' ';    // ' ' Just needs to be a non-punctuation character

        pos++;

        // Digits allowed now.
        while (pos < end && (isprint(*pos) || (0x80 & *pos)) && (!ispunct(*pos) || allowedColon == *pos) && !isspace(*pos))
        {
            if (':' == *pos) allowedColon = ' ';    // ' ' Just needs to be a non-punctuation character

            pos++;
        }

        return pos - start;
    }

    return 0;
};

long TokenMatching::RestOfLineMatcher(const char* start, const char* end)
{
    const char* pos = start;

    while (pos < end && '\n' != *pos) pos++;

    return pos - start;
};

void Tokenizer::internalTokenize(const char*& runner, const char* end)
{
    long lineNumber = 1;
    long characterNumber = 1;

    // Skip UTF8 BOM if it exists
    if ((runner + 3 <= end) && (0xEF == (unsigned char)*runner) && (0xBB == (unsigned char)*(runner + 1)) && (0xBF == (unsigned char)*(runner + 2)))
    {
        runner += 3;
    }

    while (runner < end)
    {
        tokens.emplace_back(lineNumber, characterNumber);

        // Skip over the whitespace.
        while (runner < end && isspace(*runner))
        {
            characterNumber++;
            if ('\n' == *runner++)
            {
                lineNumber++;
                characterNumber = 1;
            }
        }

        char toLookup = *runner;

        bool is_punct = ispunct(toLookup);
        // The digit 0 can be any of the nuumber encodings.
        // '1' to '9' can only be decimal encoding.  Use '1' as the lookup so we don't need to repeat the matchers.
        if ('2' <= toLookup && toLookup <= '9')
        {
            toLookup = '1';
        }
        else if ((isprint(toLookup) || (0x80 & toLookup)) && (!is_punct || ':' == toLookup) && !isspace(toLookup) && !isdigit(toLookup))
        {
            toLookup = 'a';  // Start of an itentifier - match them all to 'a'
            is_punct = false;
        }

        auto match = tokenReadingMap.find(toLookup);
        if (match != tokenReadingMap.end())
        {
            match->second->findNextToken(tokens.back(), characterNumber, lineNumber, runner, end);
        }
        else if (is_punct)
        {
            // Bad / unsupported punctuation
            Token& token = tokens.back();
            token.typeFlags = Token::badPunctuation;
            token.tokenString = string_view(runner++, 1);
            characterNumber++;
        }
        else
        {
            // Not matched - make it a bad token from the current location to the next whitespace
            // This should not happen unless the char = 0 map entry was not set.
            failTokenToNextWhitespace(tokens.back(), Token::badUnknown, characterNumber, lineNumber, runner, end);
        }
    }

    tokens.emplace_back(lineNumber, characterNumber, Token::endOfInput);
}

Tokenizer::Tokenizer(map<char, TokenMatching*>* inTokenReadingMap) :
    tokenReadingMap(*inTokenReadingMap),
    tokens(*(new token_vector()))
{}

void Tokenizer::tokenize(istream& input)
{
    auto readData = new ReadFileData();
    sourceFileData.push_back(readData);

    const char* start = readData->readInFile(input);
    internalTokenize(start, readData->end());
}

void Tokenizer::tokenize(boost::filesystem::path& filePath)
{
    auto readData = new ReadFileData();
    sourceFileData.push_back(readData);

    const char* start = readData->readInFile(filePath);
    internalTokenize(start, readData->end());
}

void Tokenizer::tokenize(string_view stringBuffer)
{
    auto readData = new ReadFileData();
    sourceFileData.push_back(readData);

    const char* start = readData->useExistingBuffer(stringBuffer.data(), stringBuffer.size());
    internalTokenize(start, readData->end());
}


void Tokenizer::cleanup()
{
    tokens.clear();

    for (auto it = sourceFileData.begin(); it != sourceFileData.end(); ++it) delete *it;
    sourceFileData.clear();
}
