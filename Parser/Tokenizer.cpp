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

map<long, string_view> Token::tokenTypeNames = map<long, string_view>({
        make_pair(notAFailure, "notAFailure"sv),
        make_pair(endOfInput, "endOfInput"sv),

        make_pair(unknownToken, "unknownToken"sv),

        make_pair(incomplete, "incomplete"sv),
        make_pair(comment, "comment"sv),
        make_pair(multiLineComment, "multiLineComment"sv),
        make_pair(identifier, "identifier"sv),
        make_pair(number, "number"sv),
        make_pair(hexNumber, "hexNumber"sv),
        make_pair(stringValue, "stringValue"sv),
        make_pair(assignment, "assignment"sv),
        make_pair(scope, "scope"sv),
        make_pair(member, "member"sv),
        make_pair(name, "name"sv),
        make_pair(keyword, "keyword"sv),
        make_pair(functionReturn, "functionReturn"sv),
        make_pair(selfCall, "selfCall"sv),
        make_pair(compilerFlag, "compilerFlag"sv),


        make_pair(packageName, "packageName"sv),

        make_pair(block, "block"sv),
        make_pair(block_start, "block_start"sv),
        make_pair(block_end, "block_end"sv),
        make_pair(params_start, "params_start"sv),
        make_pair(params_end, "params_end"sv),
        make_pair(prototype_start, "prototype_start"sv),
        make_pair(prototype_end, "prototype_end"sv),

        make_pair(failures, "failures"sv),
        make_pair(badString, "badString"sv),
        make_pair(badNumber, "badNumber"sv),
        make_pair(badPunctuation, "badPunctuation"sv),
        make_pair(badUnknown, "badUnknown"sv),
    });

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

ostream& Token::OutputTypeFlag(ostream& output)
{
    long flagRemoved = typeFlags & ~packageName;
    long packageFlag = typeFlags & packageName;

    auto match = Token::tokenTypeNames.find(flagRemoved);
    if (match != Token::tokenTypeNames.end())
    {
        output << match->second;
    }
    else
    {
        output << flagRemoved;
    }
    if (0 != packageFlag)
    {
        match = Token::tokenTypeNames.find(packageFlag);
        if (match != Token::tokenTypeNames.end())
        {
            output << " " << match->second;
        }
        else
        {
            output << " " << packageFlag;
        }
    }

    return output;
}

token_vector::token_vector() : vector<Token>()
{}

void failTokenToNextWhitespace(Token& token, long faiureCode, long& characterNumber, long lineNumber, const char*& runner, const char* end)
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


void Tokenizer::internalTokenize(const char*& runner, const char* end)
{
    long lineNumber = 1;
    long characterNumber = 1;

    // Skip UTF8 BOM if it exists
    if ((runner + 3 <= end) && (0xEF == (unsigned char)*runner) && (0xBB == (unsigned char)*(runner + 1)) && (0xBF == (unsigned char)*(runner + 2)))
    {
        runner += 3;
    }

    auto whitespaceMatch = tokenReadingMap.find(' ');
    if (whitespaceMatch != tokenReadingMap.end())
    {
        auto whitespaceMatcher = whitespaceMatch->second->tokenMatcher;
        char whiteSpaceSpecial = whitespaceMatch->second->matchOrSpecial;

        while (runner < end)
        {

            // Skip over the whitespace.
            MatchInfo info = (*whitespaceMatcher)(runner, end, whiteSpaceSpecial);

            lineNumber += info.lines;
            characterNumber += info.chars;
            runner += info.length;

            char toLookup = *runner;

            bool is_punct = ispunct(toLookup);

            // Map all digits to 0 for simplified lookup
            if (isdigit(toLookup))
            {
                toLookup = '0';
            }
            else if ((isprint(toLookup) || (0x80 & toLookup)) && (!is_punct || ':' == toLookup) && !isspace(toLookup) && !isdigit(toLookup))
            {
                toLookup = 'a';  // Start of an itentifier - match them all to 'a'
            }

            tokens.emplace_back(lineNumber, characterNumber);

            info.clear();

            auto match = tokenReadingMap.find(toLookup);
            if (match != tokenReadingMap.end())
            {
                if (NULL != *match->second->tokenMatcher)
                {
                    info = (*match->second->tokenMatcher)(runner, end, match->second->matchOrSpecial);
                }
                else
                {
                    // Single character match - matchOrSpecial goes too the inf.id
                    info.id = match->second->matchOrSpecial;
                    info.length = 1;
                    info.chars = 1;
                }
            }

            Token& token = tokens.back();
            if (0 < info.length && 0 != info.id)
            {
                token.tokenString = string_view(runner, info.length);
                (*idToTokenType)(info, token);

                runner += info.length;
                lineNumber += info.lines;
                characterNumber += info.chars;
            }
            else if (is_punct)
            {
                // Bad / unsupported punctuation
                token.typeFlags = Token::badPunctuation;
                token.tokenString = string_view(runner++, 1);
                characterNumber++;
            }
            else
            {
                // Not matched - make it a bad token from the current location to the next whitespace
                // This should not happen unless the char = 0 map entry was not set.
                failTokenToNextWhitespace(token, Token::badUnknown, characterNumber, lineNumber, runner, end);
            }
        }

        tokens.emplace_back(lineNumber, characterNumber, Token::endOfInput);

    }
}


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
