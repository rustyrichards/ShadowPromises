#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "framework.h"
#include "Tokenizer.h"
#include "SymbolTable.h"
#include <initializer_list>
#include <vector>
#include <string>
#include <unordered_set>

using namespace std;

/*
* The Goal of Parser is to turn the tokens into valid statements, or resnable error messages.
* It should:
*   1) Match the tokens to valid statements
*       a) Valid statementes are added to the parse tree
*       b) Invalid statements are formattted into an error for display
*   2) Set variable types
*       a) Untyped variables have their types set
*       b) Variable type mismatches are formatted into an error for display
*   3) The parse rules are based on state and matching tokens. These rules can be replaced
*/

enum PareseStates {
    None = 0,
    Root = 1,       // Outside of any block.
    InBlock,        // Outside of function.  InFunction takes precidence.
    InFunction,     // Inside of a functions.

    // Flags
    InLoop = 16,    // Inside of a loop (Loop exits allowed.)
};



class ParseMatch {
public:
    string_view     toMatch;
    long            tokenTypeToMatch;
    long            requiredIdentifierType;

    /*
    * In :if ... :then {...} :else {...}  the :else - the statement ends if there is no else.
    * stopIfNotMatched is set on optional part of a statement.
    * This is neded for the Map.  We need to always match the full statement, we can't have arbitrary order in potential statements.
    */
    bool            stopIfNotMatched;

    PareseStates    stateChange;
    long            stateRemoval;   // The inverse of the ParseStates flags ored together

    bool operator == (const ParseMatch& other) const
    {
        if (0 < tokenTypeToMatch) return tokenTypeToMatch == other.tokenTypeToMatch;
        if (0 < toMatch.length()) return toMatch == other.toMatch;
        if (0 < requiredIdentifierType) return requiredIdentifierType == other.requiredIdentifierType;
    }

    bool matches(const Token& token, long (*symbolTableTypeMatcher)(const Token& token) ) const
    {
        if (0 < tokenTypeToMatch) return tokenTypeToMatch == token.typeFlags;
        if (0 < toMatch.length()) return toMatch == token.tokenString;

        long type = symbolTableTypeMatcher(token);
        if (0 < requiredIdentifierType && requiredIdentifierType <= Symbol::specifficType)
        {
            return requiredIdentifierType == (type & Symbol::specifficType);
        }
        else if (requiredIdentifierType > Symbol::specifficType)
        {
            return requiredIdentifierType == (type & ~Symbol::specifficType);
        }
    }

    bool operator < (const ParseMatch& other) const
    {
        if (0 < tokenTypeToMatch) return tokenTypeToMatch < other.tokenTypeToMatch;
        if (0 < toMatch.length()) return 0 < toMatch.compare(other.toMatch);
        if (0 < requiredIdentifierType) return requiredIdentifierType < other.requiredIdentifierType;
    }

    // Use tokenTypeToMatch in preference to toMatch (string_view) it is faster, and simplier
    ParseMatch(long inTokenType) :
        toMatch(""sv),
        tokenTypeToMatch(inTokenType),
        stopIfNotMatched(false),
        stateChange(None),
        stateRemoval(-1)
    {}

    ParseMatch(string_view inToMatch, long inTokenType = Token::unknownToken) :
        toMatch(inToMatch),
        tokenTypeToMatch(inTokenType),
        stopIfNotMatched(false),
        stateChange(None),
        stateRemoval(-1)
    {}
};

class ParseStatement : public vector<ParseMatch*> {
public:
    const ParseMatch& startingMatch() const
    {
        return *front();
    }

    bool operator < (const ParseStatement& other) const
    {
        return startingMatch() < other.startingMatch();
    }

    ParseStatement(std::initializer_list<ParseMatch*> matchers)
        : vector<ParseMatch*>(matchers)
    {
    }

};

class ParseErrr {
    string errorMessage;
    Token& failingToken;

    ParseErrr(ParseStatement* failedStatement, ParseMatch* failedMatch, long parseState, Token& inToken) :
        errorMessage(),
        failingToken(inToken)
    {
        for (auto parseMatch : *failedStatement)
        {
            if (NULL != parseMatch)
            {
                if (0 < errorMessage.length()) errorMessage += "\n";

                if (0 < parseMatch->toMatch.length())
                {
                    errorMessage += parseMatch->toMatch;
                }
                else if (0 < parseMatch->tokenTypeToMatch)
                {
                    errorMessage += "Token:";
                    TokenFlagToString(errorMessage, parseMatch->tokenTypeToMatch);
                }
                else if (0 < parseMatch->requiredIdentifierType)
                {
                    errorMessage += "Type:";
                    //TypeToString(errorMessage, parseMatch->tokenTypeToMatch);
                }
            }

            if (parseMatch == failedMatch)
            {
                errorMessage += "\n-----\n";
            }

        }
        errorMessage += failingToken.tokenString;
        errorMessage += "\nLine: ";
        errorMessage += to_string(failingToken.startingLine);
        errorMessage += "\nChar: ";
        errorMessage += to_string(failingToken.startingCharacter);
    }
};


class Parser {
public:
    unordered_set<ParseStatement*> possibleStatements;

    Parser(std::initializer_list<ParseStatement*> statements)
        :possibleStatements(statements.begin(), statements.end())
    {
    }

    list<ParseErrr> parse(token_vector& tokens);
};

#endif // PARSER_H_INCLUDED
