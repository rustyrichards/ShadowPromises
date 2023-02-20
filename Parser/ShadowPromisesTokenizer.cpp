#include "pch.h"
#include <string>
#include <fstream>      // ifstream
#include <array>
#include <map>
#include <list>

#include <iostream>

void shadowPromisesIdToTokenType(const MatchInfo& info, Token& token)
{

}

extern "C++" EXPORT Tokenizer& initShadowPromisesTokenizer()
{
    Tokenizer* spTokenizer = (
        new Tokenizer(
            new map<char, TokenMatching*>({
                make_pair(' ',
                    new TokenMatching(WhiteSpaceMatcher)
                ),
                make_pair('"',
                    new TokenMatching(StartEndMatcher, '"')
                ),
                make_pair('\'',
                    new TokenMatching(StartEndMatcher, '\'')
                ),
                make_pair('0',
                    new TokenMatching(NuberMatcher)
                ),
                make_pair('-',
                    new TokenMatching(NuberMatcher)
                ),
                make_pair('/',
                    new TokenMatching(LineCommentMatcher, '/')
                ),
                make_pair('*',
                    new TokenMatching(StartEndMatcher, '*')
                ),
                make_pair('a',
                    new TokenMatching(IdentifierMatcher, ':')
                ),
                make_pair('.', new TokenMatching('.')),
                make_pair('{', new TokenMatching('{')),
                make_pair('}', new TokenMatching('}')),
                make_pair('(', new TokenMatching('(')),
                make_pair(')', new TokenMatching(')')),
                make_pair('[', new TokenMatching('[')),
                make_pair(']', new TokenMatching(']')),
                make_pair('@', new TokenMatching('@')),
            }),
            // The id (char) to typeFlags converter
            [](const MatchInfo& info, Token& token)
            {
                switch (info.id)
                {
                case '"':
                case '\'':
                    token.typeFlags = Token::stringValue;
                    break;
                case '1':
                    token.typeFlags = Token::number;
                    break;
                case 'x':
                    token.typeFlags = Token::hexNumber;
                    break;
                case '/':
                    token.typeFlags = Token::comment;
                    break;
                case '*':
                    token.typeFlags = Token::multiLineComment;
                    break;
                case 'a':
                    token.typeFlags = Token::identifier;
                    break;
                case ':':
                    token.typeFlags = Token::identifier | Token::packageName;
                    break;
                case '.':
                    token.typeFlags = Token::member;
                    break;
                case '{':
                    token.typeFlags = Token::block_start;
                    break;
                case '}':
                    token.typeFlags = Token::block_end;
                    break;
                case '(':
                    token.typeFlags = Token::params_start;
                    break;
                case ')':
                    token.typeFlags = Token::params_end;
                    break;
                case '[':
                    token.typeFlags = Token::prototype_start;
                    break;
                case ']':
                    token.typeFlags = Token::prototype_end;
                    break;
                case '@':
                    token.typeFlags = Token::assignment;
                    break;
                }
            }
        )
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
            "Type: ";
        runner->OutputTypeFlag(output) << endl << endl;

        runner++;
    }
}
