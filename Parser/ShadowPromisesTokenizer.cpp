#include "pch.h"
#include <string>
#include <fstream>      // ifstream
#include <array>
#include <map>
#include <list>

#include <iostream>

extern "C++" EXPORT Tokenizer& initShadowPromisesTokenizer()
{
    /* Changing out the tokenizer for a regex based generral tokenizer
    Tokenizer* spTokenizer = (new Tokenizer(
        new map<string_view, Token::TypeAndParsingFlags>({
            make_pair(":struct"sv, 
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::blockFollows
                )),
            make_pair(":test"sv, 
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::parametersFollows |
                    Token::ParsingFlags::testResult
                )),
            make_pair(":if"sv, 
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::requiresTestResult |
                    Token::ParsingFlags::blockFollows |
                    Token::ParsingFlags::elseAllowed
                )),
            make_pair(":else"sv, 
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::requiresElseAllowed |
                    Token::ParsingFlags::blockFollows
                )),
            make_pair(":loop"sv, 
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::loopBlock |
                    Token::ParsingFlags::blockFollows
                )),
            make_pair(":exit"sv, 
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::requiresLoopBlock |
                    Token::ParsingFlags::requiresTestResult
                )),
            make_pair(":and"sv, 
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::allowedInParameters |
                    Token::ParsingFlags::parametersFollows
                )),
            make_pair(":nand"sv,
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::allowedInParameters |
                    Token::ParsingFlags::parametersFollows
                )),
            make_pair(":or"sv,
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::allowedInParameters |
                    Token::ParsingFlags::parametersFollows
                )),
            make_pair(":xor"sv,
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::allowedInParameters |
                    Token::ParsingFlags::parametersFollows
                )),
            make_pair(":test"sv,
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::nameFollows |
                    Token::ParsingFlags::blockFollows
                )),
            make_pair(":return"sv,
                Token::TypeAndParsingFlags(
                    Token::functionReturn,
                    Token::ParsingFlags::requiresFunctionBlock |
                    Token::ParsingFlags::identifierFollows
                )),
            make_pair(":self"sv,
                Token::TypeAndParsingFlags(
                    Token::selfCall,
                    Token::ParsingFlags::requiresFunctionBlock
                )),
            make_pair(":option"sv,
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::compileFlagFollows |
                    Token::ParsingFlags::blockFollows
                )),
            make_pair(":define"sv,
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::compileFlag
                )),
            make_pair(":undefine"sv,
                Token::TypeAndParsingFlags(
                    Token::keyword,
                    Token::ParsingFlags::compileFlag
                )),
            make_pair("<"sv,
                Token::TypeAndParsingFlags(
                    Token::prototype_start,
                    Token::functionDefinition | 
                    Token::ParsingFlags::blockFollows
                    )),
            make_pair(">"sv,
                Token::TypeAndParsingFlags(Token::prototype_end)),
            make_pair("{"sv,
                Token::TypeAndParsingFlags(Token::block_start)),
            make_pair("}"sv, 
                Token::TypeAndParsingFlags(Token::block_end)),
            make_pair("("sv, 
                Token::TypeAndParsingFlags(Token::params_start)),
            make_pair(")"sv, 
                Token::TypeAndParsingFlags(Token::params_end)),
            make_pair("|"sv, 
                Token::TypeAndParsingFlags(
                    Token::assignment,
                    Token::ParsingFlags::valueFollows |
                    Token::ParsingFlags::identifierFollows
                )),
            })));
    spTokenizer->scopeChar = L':';
    */
    Tokenizer* spTokenizer = (new Tokenizer(
        new map<char, TokenMatching*>({
            make_pair('"',
                new TokenMatching(
                    TokenMatching::StringMatcher,
                    Token::stringValue,
                    Token::badString
                )
            ),
            make_pair('\'',
                new TokenMatching(
                    TokenMatching::StringMatcher,
                    Token::stringValue,
                    Token::badString
                )
            ),
            make_pair('0',
                new TokenMatching(
                    new TokenMatching(
                        TokenMatching::HexMatcher,
                        Token::number,
                        Token::badNumber
                    ),
                    TokenMatching::DecimalMatcher,
                    Token::hexNumber
                )
            ),
            make_pair('1',
                new TokenMatching(
                    TokenMatching::DecimalMatcher,
                    Token::number,
                    Token::badNumber
                )
            ),
            make_pair('-',
                new TokenMatching(
                    TokenMatching::DecimalMatcher,
                    Token::number,
                    Token::badNumber
                )
            ),
            make_pair('#',
                new TokenMatching(
                    TokenMatching::RestOfLineMatcher,
                    Token::comment,
                    Token::badUnknown
                )
            ),
            make_pair(':', new TokenMatching(Token::scope)),
            make_pair('.', new TokenMatching(Token::member)),
            make_pair('{', new TokenMatching(Token::block_start)),
            make_pair('}', new TokenMatching(Token::block_end)),
            make_pair('(', new TokenMatching(Token::params_start)),
            make_pair(')', new TokenMatching(Token::params_end)),
            make_pair('[', new TokenMatching(Token::prototype_start)),
            make_pair(']', new TokenMatching(Token::prototype_end)),
            make_pair('|', new TokenMatching(Token::assignment)),
            make_pair(0,
                new TokenMatching(
                    TokenMatching::IdentifierMatcher,
                    Token::comment,
                    Token::badUnknown
                )
            ),
        }))
    );
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
