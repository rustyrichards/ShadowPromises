#include "pch.h"
#include <string>
#include <fstream>      // ifstream
#include <array>
#include <map>
#include <list>

#include "ShadowPromisesTokenizer.h"
#include <iostream>

extern "C" EXPORT Tokenizer & initShadowPromisesTokenizer()
{
    Tokenizer* spTokenizer = (new Tokenizer(
        new map<string_view, Token::TypeAndParsingFlags>({
            make_pair(":function"sv, 
                Token::TypeAndParsingFlags(Token::TokenType::keyword)),
            make_pair(":struct"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::needsBlock
                )),
            make_pair(":test"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::needsParameters |
                    Token::ParsingFlags::testResult
                )),
            make_pair(":if"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::needsTestResult |
                    Token::ParsingFlags::needsBlock |
                    Token::ParsingFlags::elseAllowed
                )),
            make_pair(":else"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::needsElseAllowed |
                    Token::ParsingFlags::needsBlock
                )),
            make_pair(":loop"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::loopBlock |
                    Token::ParsingFlags::needsBlock
                )),
            make_pair(":exit"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::needsLoopBlock |
                    Token::ParsingFlags::needsTestResult
                )),
            make_pair(":and"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::needsParameters
                )),
            make_pair(":or"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::needsParameters
                )),
            make_pair("{"sv, 
                Token::TypeAndParsingFlags(Token::TokenType::block_start)),
            make_pair("}"sv, 
                Token::TypeAndParsingFlags(Token::TokenType::block_end)),
            make_pair("("sv, 
                Token::TypeAndParsingFlags(Token::TokenType::params_start)),
            make_pair(")"sv, 
                Token::TypeAndParsingFlags(Token::TokenType::params_end)),
            make_pair("|"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::assignment,
                    Token::ParsingFlags::needsValue |
                    Token::ParsingFlags::needsIdentifier
                )),
            })));
    spTokenizer->scopeChar = L':';

    return *spTokenizer;
}

extern "C" EXPORT void dumpTokens(
    ostream& output, 
    vector<Token> tokens)
{
    auto runner = tokens.begin();
    int count = 0;
    while (runner != tokens.end())
    {
        output << "TokenIndex:  " << count++ << "  String: " << runner->tokenString << endl <<
            "Line: " << runner->startingLine << "  Characer: " << runner->startingCharacter << endl <<
            "Type: " << runner->typeAndFlags.type << endl <<
            "ParseFlags: " << runner->typeAndFlags.parsingFlags << endl << endl;
        runner++;
    }
}

extern "C" EXPORT void handleCommandlineAndTokenize(
    Tokenizer& shadowPromisesTokenizer, 
    istream& initialInput, 
    ostream& output,
    const char* inputPrompt,
    int argc, 
    char* argv[], 
    char* envp[])
{
    bool wasInputFileFound = false;


    if (argc > 1)
    {
        ifstream fileInput;

        char* option = NULL;
        for (int i = 1; i < argc; i++)
        {
            if (fileInput.is_open()) fileInput.close();

            if (L'-' == argv[i][0])
            {
                option = argv[i] + 1;
            }
            else
            {
                try
                {
                    boost::filesystem::path testPath(argv[i]);
                    output << "Tokenizing \"" << argv[i] << "\"" << endl << endl;

                    shadowPromisesTokenizer.tokenize(testPath);
                    wasInputFileFound = true;
                }
                catch (exception& ex)
                {
                    output << "Could not open \"" << argv[i] << "\" as a file.  " <<
                        ex.what();
                }
            }
        }
    }

    if (!wasInputFileFound)
    {
        if (NULL != inputPrompt) output << inputPrompt << endl;

        shadowPromisesTokenizer.tokenize(initialInput);
    }
}