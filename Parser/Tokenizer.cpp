#include "pch.h"

#include <iostream>
#include <fstream>      // ifstream

bool Token::isKeyword()
{
    return keyword <= typeAndFlags.type && typeAndFlags.type < keyword + sectionSize;
}

bool Token::isIdentifier()
{
    return identifier <= typeAndFlags.type && typeAndFlags.type < identifier + sectionSize;
}

bool Token::isBlock()
{
    return block <= typeAndFlags.type && typeAndFlags.type < block + sectionSize;
}

void Token::init(
    long inStartingLine, 
    long inStartingCharacter, 
    int inType,
    int inParsingFlags)
{
    hasScope = false;
    startingLine = inStartingLine;
    startingCharacter = inStartingCharacter;
    tokenString = ""sv;
    typeAndFlags.type = inType;
    typeAndFlags.parsingFlags = inParsingFlags;
}

Token::Token(
    long inStartingLine,
    long inStartingCharacter,
    int inType,
    int inParsingFlags) :
    hasScope(false),
    startingLine(inStartingLine),
    startingCharacter(inStartingCharacter),
    tokenString(""sv),
    typeAndFlags(inType, inParsingFlags)
{}


long Token::checkHexNumberToken()
{
    if (2 < tokenString.length())
    {
        auto iter = tokenString.begin();
        if ('0' == *(iter++)) {
            if ('X' == *iter || 'x' == *iter)
            {
                typeAndFlags.type = Token::hexNumber;
                while (++iter < tokenString.end()) 
                {
                    if (!isdigit(*iter) &&
                        ('a' > *iter || *iter > 'f') &&
                        ('A' > *iter || *iter > 'F'))
                    {
                        typeAndFlags.type = Token::badNumber;
                        break;
                    }
                }
            }
        }
    }
    return typeAndFlags.type;
}

// NOTE: checkNumberToken returns typeAndFlags.type):
// If typeAndFlags.type is something other than Token::unknownToken => return out and do not test
// Otherwise
//          Token::number => A valid number
//          Token::badNumber => starts wiith a number prefix ('.', '-' or a digit) but is not valid
//          Token::unknownToken => something not a number
long Token::checkNumberToken()
{
    if (Token::unknownToken != typeAndFlags.type ||
        Token::unknownToken != checkHexNumberToken())
    {
        return  typeAndFlags.type;
    }

    auto iter = tokenString.begin();

    bool dotAllowed = true;
    bool eAllowed = true;
    char lastChr = 0;

    while (iter != tokenString.end())
    {
        if (*iter == L'.')
        {
            // "." ia a bad number, so is "-.". Both will be corrected by a following digit
            if (0 == lastChr || '-' == lastChr) typeAndFlags.type = Token::badNumber;

            if (!dotAllowed)
            {
                // multiple '.' or '.' after 'e' not a number
                typeAndFlags.type = Token::badNumber;
                break;
            }
            dotAllowed = false;
        }
        else if (*iter == L'e' || *iter == L'e')
        {
            if (0 == lastChr) 
            {
                // starts with 'e' not a number
                typeAndFlags.type = Token::unknownToken;
                break;
            }

            // 'e' not followed by a digit is bad.
            typeAndFlags.type = Token::badNumber;

            if (!eAllowed || (!isdigit(lastChr) && lastChr != '.'))
            {
                // multiple 'e' is invalid, 'e' after anything other than a digit or '.' is invalid
                break;
            }

            eAllowed = false;   // There can only be one 'e' exponential, 
            dotAllowed = false; // No more '.' after the exponent
        }
        else if (*iter == '-')
        {
            // '-' not followed by a digit is bad.
            typeAndFlags.type = Token::badNumber;

            if (0 != lastChr && 'e' != lastChr && 'E' != lastChr)
            {
                // '-' is only allowed at the start or right after 'e'
                break;
            }
        }
        else if (isdigit(*iter))
        {
            // If we have encountered a digit, this is a valid potential number end - Valid numbers must always end with a digit.
            typeAndFlags.type = Token::number;
        }
        else
        {
            typeAndFlags.type = (0 == lastChr)
                ? Token::unknownToken // does not start with a number character - not a number token
                : Token::badNumber; // starts with a number character - then something not a number - not valid.
            break;
        }
        lastChr = *(iter++);
    }

    return typeAndFlags.type;
}


token_vector::token_vector() : vector<Token>()
{}

Tokenizer::Tokenizer(map<string_view, Token::TypeAndParsingFlags>* inKeywordTokens) :
    allowDoubleQuoteStrings(true),
    allowSingleQuoteStrings(true),
    scopeChar(0),
    keywordTokens(*inKeywordTokens),
    tokens(*(new token_vector()))
{}



void Tokenizer::finishToken(
    Token& token,
    const char*& start,
    const char* end,
    int inType)
{
    token.typeAndFlags.type = inType;

    token.tokenString = string_view(&*start, end - start);

    if (Token::unknownToken == token.checkNumberToken())
    {
        auto foundItem = keywordTokens.find(token.tokenString);
        if (keywordTokens.end() != foundItem)
        {
            token.typeAndFlags = foundItem->second;
        }
        else
        {
            // Identifiers supply their value
            token.typeAndFlags.type = Token::identifier;
            token.typeAndFlags.parsingFlags = Token::ParsingFlags::value;
        }
    }
    // else the token.typeAndFlags is set.

    start = end;
}

void Tokenizer::readInComment(
    Token& token,
    const char*& runner,
    const char* end,
    long& characterCount)
{
    token.typeAndFlags.type = Token::comment;

    const char* start = runner;

    while (++runner < end && '\r' != *runner && '\n' != *runner);

    characterCount += runner - start;
    token.tokenString = string_view(start, runner - start);
}

bool Tokenizer::readInString(
    char stringStart,
    const char*& runner,
    const char* end,
    long& lineCount,
    long& characterCount,
    bool multilineString)
{
    bool isGoodString = false;
    while (runner < end)
    {
        bool notEscaped = L'\\' != *runner;
        characterCount++;
        runner++;
        if (runner < end)
        {
            if ('\r' == *runner && !multilineString)
            {
                break;  // End of line before end of string - badString
            }
            else if ('\n' == *runner)
            {
                if (multilineString)
                {
                    // Line end in a multi line string
                    lineCount++;
                    characterCount = 1;
                }
                else
                {
                    break;  // End of line before end of string - badString
                }
            }
            else if (notEscaped && stringStart == *runner)
            {
                isGoodString = true;
                runner++;
                break;
            }
        }
    }

    return isGoodString;
}

// handleCurrentCharacterOverride must increment runner if it advances!
bool Tokenizer::handleCurrentCharacterOverride(
    Token& token,
    const char*& start,
    const char*& runner,
    const char* end,
    long& lineCount,
    long& characterCount)
{
    return false;
}


void Tokenizer::handleCurrentCharacter(
    Token& token,
    const char*& start,
    const char*& runner,
    const char* end,
    long& lineCount,
    long& characterCount)
{
    char current = *runner;

    // switch can preform better then the lots of if(s)
    if (!handleCurrentCharacterOverride(token, start, runner, end, lineCount, characterCount))
    {
        switch (current)
        {
        case L'#':
            if (start != runner)
            {
                // The token was started. save that then pickup the comment
                finishToken(token, start, runner);
            }
            // Read in the  line comment
            readInComment(token, runner, end, characterCount);
            start = runner;
            break;
        case L'\n':
        case L'\r':
        case L' ':
        case L'\t':
        case L'\v':
            characterCount++;
            if (start == runner)
            {
                //  If the token is not started skip the whitespace
                start++;
                runner++;
                token.startingCharacter = characterCount;

                // If we have reached the line end go the the next.
                if (L'\n' == current)
                {
                    // The line advanced before the token start
                    token.startingLine = ++lineCount;
                    token.startingCharacter = (characterCount = 1);
                }
            }
            else
            {
                // The token is started.  The whitespace ends the token.
                finishToken(token, start, runner);
            }
            break;
        case L'+':  // +" of +' is multi line strings
            if (start != runner)
            {
                finishToken(token, start, runner);
            }

            //  Read in the multi-line string
            characterCount++;
            runner++;
            if (runner < end)
            {
                char stringStart = *runner;
                if ((allowDoubleQuoteStrings || L'"' != stringStart) &&
                    (allowSingleQuoteStrings || L'\'' != stringStart))
                {
                    bool isValidString = readInString(stringStart, runner, end, lineCount, characterCount, true /* Multiline */);
                    finishToken(token, start, runner, (isValidString) ? Token::string : Token::badString);
                }
                else
                {
                    // + without the following string character is badPunctuation
                    token.typeAndFlags.type = Token::badPunctuation;
                    finishToken(token, start, runner);
                }
            }
        break;
        case L'\'':
        case L'"':
            // Trick - fall through to the punctuation if the quote is not supported.
            if ((allowDoubleQuoteStrings || L'"' != current) &&
                (allowSingleQuoteStrings || L'\'' != current))
            {
                characterCount++;
                if (start != runner)
                {
                    // The token is started.  The quote ends the token.
                    finishToken(token, start, runner);
                }

                // Read in the string token
                bool isValidString = readInString(current, runner, end, lineCount, characterCount);
                finishToken(token, start, runner, (isValidString) ? Token::string : Token::badString);
                break;
            }
            // else unsupported quote will fall throught to punctuation

        // Basically these punctuation characters are ispunct except for 
        //  '_' which is valid in identifiers,
        //  '-' is valid starting numbers and exponents, and can be other than the first characer in an identifier
        //  '#' for comments, '"' and '\'' for strings,
        //  ',' for numbers
        case L'!':
        case L'$':
        case L'%':
        case L'&':
        case L'(':
        case L')':
        case L'*':
        case L',':
        case L'/':
        case L':':
        case L';':
        case L'<':
        case L'=':
        case L'>':
        case L'?':
        case L'@':
        case L'[':
        case L']':
        case L'^':
        case L'`':
        case L'{':
        case L'|':
        case L'}':
        case L'~':
            if (scopeChar == current)
            {
                token.hasScope = true;

                // A token character but not a complete.
                runner++;
                characterCount++;
            }
            else
            {
                characterCount++;
                if (start == runner)
                {
                    //  If the token is not started this punctuation will be the token
                    runner++;
                }
                // else - The text before the punctuation is the token
                finishToken(token, start, runner);
            } 
            break;
        default:
            // A token character, but not complete.
            runner++;
            characterCount++;
        }
    }
}


const char* Tokenizer::findNextToken(
    Token& token,
    const char* start,
    const char* end,
    long& lineCount,
    long& characterCount)
{
    token.startingLine = lineCount;
    token.startingCharacter = characterCount;

    const char* runner = start;

    while (token.typeAndFlags.type == Token::incomplete && runner != end)
    {
        handleCurrentCharacter(token, start, runner, end, lineCount, characterCount);
    }

    return runner;
}



void Tokenizer::internalTokenize(const char* start, const char* end)
{
    long lineNumber = 1;
    long characterNumber = 1;

    bool wasLastIncomplete = false;

    while (start < end)
    {
        tokens.emplace_back(lineNumber, characterNumber);

        start = findNextToken(tokens.back(), start, end, lineNumber, characterNumber);

        wasLastIncomplete = Token::incomplete == tokens.back().typeAndFlags.type;

    }

    // If the current token is incomplete and empty - make it the endOfInput
    // NOTE: wasLastIncomplete won't be true unless tokens.back() exists!
    if (wasLastIncomplete && Token::incomplete == tokens.back().typeAndFlags.type && tokens.back().tokenString.length() == 0)
    {
        tokens.back().init(lineNumber, characterNumber, Token::endOfInput);
    }
    else
    {
        tokens.emplace_back(lineNumber, characterNumber, Token::endOfInput);
    }
}

void Tokenizer::tokenize(istream& input)
{
    auto readData = new ReadFileData();
    sourceFileData.push_back(readData);

    const char* start = readData->readInFile(input);
    Tokenizer::internalTokenize(start, readData->end());
}

void Tokenizer::tokenize(boost::filesystem::path& filePath)
{
    auto readData = new ReadFileData();
    sourceFileData.push_back(readData);

    const char* start = readData->readInFile(filePath);
    Tokenizer::internalTokenize(start, readData->end());
}

void Tokenizer::tokenize(string_view stringBuffer)
{
    auto readData = new ReadFileData();
    sourceFileData.push_back(readData);

    const char* start = readData->useExistingBuffer(stringBuffer.data(), stringBuffer.size());
    Tokenizer::internalTokenize(start, readData->end());
}


void Tokenizer::cleanup()
{
    tokens.clear();

    for (auto it = sourceFileData.begin(); it != sourceFileData.end(); ++it) delete *it;
    sourceFileData.clear();
}