#include "pch.h"

Token emptyToken(0L, 0L, Token::nonToken);
string emptyString = "";

Symbol emptySymbol(emptyToken, emptyString);

Symbol SymbolTable::findTokenDefinition(Symbol toFind)
{
    Symbol match = emptySymbol;

    auto pos = symbols.lower_bound(toFind);

    if (pos != symbols.end())
    {
        if (*pos == toFind) match = *pos;  // matched the same
        --pos;
    }
    if (pos != symbols.end() && pos->matchSymbolAndLocation(toFind))
    {
        return *pos;
    }

    return match;
}

Symbol SymbolTable::addOrMatchSymbol(Token& token, string& location)
{
    Symbol newSymbol(token, location);

    Symbol matched = findTokenDefinition(newSymbol);
    if (matched.token.typeFlags != Token::nonToken)
    {
        return matched;
    }
    else
    {
        // Location is free to change after this function ends.  So we will make a backup of the location.
        stringsToCleanup.push_back(new string(location));
        newSymbol.parseTreeLocation = *(stringsToCleanup.back());

        symbols.emplace(newSymbol);
        return newSymbol;
    }
}
