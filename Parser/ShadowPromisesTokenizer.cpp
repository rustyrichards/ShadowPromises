#include "pch.h"
#include <string>
#include <fstream>      // ifstream
#include <array>
#include <map>
#include <list>

#include <iostream>

extern "C++" EXPORT Tokenizer& initShadowPromisesTokenizer()
{
    Tokenizer* spTokenizer = (new Tokenizer(
        new map<string_view, Token::TypeAndParsingFlags>({
            make_pair(":struct"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::blockFollows
                )),
            make_pair(":test"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::parametersFollows |
                    Token::ParsingFlags::testResult
                )),
            make_pair(":if"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::requiresTestResult |
                    Token::ParsingFlags::blockFollows |
                    Token::ParsingFlags::elseAllowed
                )),
            make_pair(":else"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::requiresElseAllowed |
                    Token::ParsingFlags::blockFollows
                )),
            make_pair(":loop"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::loopBlock |
                    Token::ParsingFlags::blockFollows
                )),
            make_pair(":exit"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::requiresLoopBlock |
                    Token::ParsingFlags::requiresTestResult
                )),
            make_pair(":and"sv, 
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::allowedInParameters |
                    Token::ParsingFlags::parametersFollows
                )),
            make_pair(":nand"sv,
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::allowedInParameters |
                    Token::ParsingFlags::parametersFollows
                )),
            make_pair(":or"sv,
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::allowedInParameters |
                    Token::ParsingFlags::parametersFollows
                )),
            make_pair(":xor"sv,
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::allowedInParameters |
                    Token::ParsingFlags::parametersFollows
                )),
            make_pair(":test"sv,
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::nameFollows |
                    Token::ParsingFlags::blockFollows
                )),
            make_pair(":return"sv,
                Token::TypeAndParsingFlags(
                    Token::TokenType::functionReturn,
                    Token::ParsingFlags::requiresFunctionBlock |
                    Token::ParsingFlags::identifierFollows
                )),
            make_pair(":self"sv,
                Token::TypeAndParsingFlags(
                    Token::TokenType::selfCall,
                    Token::ParsingFlags::requiresFunctionBlock
                )),
            make_pair(":option"sv,
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::compileFlagFollows |
                    Token::ParsingFlags::blockFollows
                )),
            make_pair(":define"sv,
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::compileFlag
                )),
            make_pair(":undefine"sv,
                Token::TypeAndParsingFlags(
                    Token::TokenType::keyword,
                    Token::ParsingFlags::compileFlag
                )),
            make_pair("<"sv,
                Token::TypeAndParsingFlags(
                    Token::TokenType::prototype_start,
                    Token::functionDefinition | 
                    Token::ParsingFlags::blockFollows
                    )),
            make_pair(">"sv,
                Token::TypeAndParsingFlags(Token::TokenType::prototype_end)),
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
                    Token::ParsingFlags::valueFollows |
                    Token::ParsingFlags::identifierFollows
                )),
            })));
    spTokenizer->scopeChar = L':';

    return *spTokenizer;
}

extern "C" EXPORT void dumpTokens(
    ostream& output, 
    token_vector tokens)
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
