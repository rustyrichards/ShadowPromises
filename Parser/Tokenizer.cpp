#include "pch.h"
#include "Tokenizer.h"

#include <iostream>
#include <fstream>      // ifstream

bool Token::IsKeyword()
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

void Token::Init(
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
    tokenString(""),
    typeAndFlags(inType, inParsingFlags)
{}

Token::TypeAndParsingFlags::TypeAndParsingFlags(
    int inType,
    int inFlags)
{
    type = inType;
    parsingFlags = inFlags;
}


Tokenizer::Tokenizer(map<string_view, Token::TypeAndParsingFlags>* inKeywordTokens) :
    allowDoubleQuoteStrings(true),
    allowSingleQuoteStrings(true),
    scopeChar(0),
    keywordTokens(*inKeywordTokens),
    tokens(*(new vector<Token>())),
    tokenRecovery(*(new vector<Token>()))
{}

bool Tokenizer::testIsNumber(string_view& maybeNumber)
{
    auto iter = maybeNumber.begin();

    bool wasDotFound = false;
    while (iter != maybeNumber.end())
    {
        if (*iter == L'.')
        {
            if (wasDotFound) return false;  // multiple '.' - not a number
            wasDotFound = true;
        }
        else if (!iswdigit(*iter))
        {
            return false;   // not all digits and '.'
        }
        iter++;
    }

    // "." is not a number!
    return maybeNumber.length() > 1 || !wasDotFound;
}



void Tokenizer::finishToken(
    Token& token,
    const char*& start,
    const char* end,
    int inType)
{
    token.typeAndFlags.type = inType;

    token.tokenString = string_view(&*start, end - start);

   if (Token::unknown == token.typeAndFlags.type)
    {
        if (L'#' == *start)
        {
            token.typeAndFlags.type = Token::comment;
            token.typeAndFlags.parsingFlags = Token::ParsingFlags::none;

            const char* runner = start;
            do
            {
                runner++;

            } while (L'\r' != *runner && L'\n' != *runner && runner < end);
            token.tokenString = string_view(&*start, runner - start);

            // Now advance past the token we made.
            start = runner;
        }
        else if (testIsNumber(token.tokenString))
        {
            token.typeAndFlags.type = Token::number;
            // numbers supply their value
            token.typeAndFlags.parsingFlags = Token::ParsingFlags::value;
        }
        else
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
    }
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

    // switch can be preform better then the lots of if(s)
    if (!handleCurrentCharacterOverride(token, start, runner, end, lineCount, characterCount))
    {
        switch (current)
        {
        case L'#':
            if (start == runner)
            {
                // If the token is not started this is a line comment
                finishToken(token, runner, end);

                // In most cases the line will end, and we will just go back to characterCount 1.
                // Changing the characterCount here matters for the endOfInput!
                characterCount += (long)token.tokenString.length();
            }
            else
            {
                // The token was started. save that then pickup the comment
                finishToken(token, start, runner);
            }
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
        case L'\'':
        case L'"':
            // Trick - fall through to the punctuation if the quote is not supported.
            if ((allowDoubleQuoteStrings || L'"' != current) &&
                (allowSingleQuoteStrings || L'\'' != current))
            {
                characterCount++;
                if (start == runner)
                {
                    bool isValidString = readInString(current, runner, end, lineCount, characterCount);
                    finishToken(token, start, runner, (isValidString) ? Token::string : Token::badString);
                }
                else
                {
                    // The token is started.  The quote ends the token.
                    finishToken(token, start, runner);
                }
                break;
            }
            // else unsupported quote will fall throught to punctuation

        case L'+':  // +" of +' is multi line strings
            characterCount++;
            if (start == runner)
            {
                //  If the token is not started this punctuation will be the token
                runner++;
                if (runner != end)
                {
                    char stringStart = *runner;
                    if ((allowDoubleQuoteStrings || L'"' != stringStart) &&
                        (allowSingleQuoteStrings || L'\'' != stringStart))
                    {
                        bool isValidString = readInString(stringStart, runner, end, lineCount, characterCount, true /* Multiline */);
                        finishToken(token, start, runner, (isValidString) ? Token::string : Token::badString);
                    }
                }
            }
            else    // The text before the punctuation is the token
            {
                finishToken(token, start, runner);
            }
            break;

        // Basically these punctuation characters are ispunct except for 
        //  '_' which is valid in identifiers, 
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
        case L'-':
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

    // End of line ends the token
    if (token.typeAndFlags.type == Token::incomplete && runner == end && start != runner && !iswspace(*start))
    {
        finishToken(token, start, end);
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
        if (tokens.empty() || Token::incomplete != tokens.back().typeAndFlags.type)
        {
            if (!tokenRecovery.empty())
            {
                Token& toMove = tokenRecovery.back();
                tokenRecovery.pop_back(); 
                toMove.Init(lineNumber, characterNumber);
                tokens.push_back(toMove);
            }
            else
            {
                tokens.emplace_back(lineNumber, characterNumber);
            }
        }

        start = findNextToken(tokens.back(), start, end, lineNumber, characterNumber);

        wasLastIncomplete = Token::incomplete == tokens.back().typeAndFlags.type;

    }

    // If the current token is incomplete and empty - make it the endOfInput
    // NOTE: wasLastIncomplete won't be true unless tokens.back() exists!
    if (wasLastIncomplete && Token::incomplete == tokens.back().typeAndFlags.type && tokens.back().tokenString.length() == 0)
    {
        tokens.back().Init(lineNumber, characterNumber, Token::endOfInput);
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

void Tokenizer::tokenize(boost::filesystem::path filePath)
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
    auto runner = tokens.begin();
    auto end = tokens.end();
    while (runner != end)
    {
        (runner++)->Init(1, 1);
    }
    if (0 <tokens.size()) tokenRecovery.assign(tokens.begin(), tokens.end());
    
    tokens.clear();

    for (auto it = sourceFileData.begin(); it != sourceFileData.end(); ++it) delete *it;
    sourceFileData.clear();
}