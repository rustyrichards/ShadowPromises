#pragma once

#include <set>
#include <string>
#include "Tokenizer.h"

using namespace std;

/*
* This symbol table stores and matches identifiers.
*/

class FunctionPrototype;
class StructureType;

class Symbol
{
public:
    enum SymbolTypes {
        // first the generic types
        undefined = 0,
        logical,
        number,
        strType,
        function,
        structure,

        // Speciffic types
        specifficType = 7,
        n_int = specifficType,
        n_float,
        f_type,     // The function types go up by 2 from here (odd)
        s_type,     // The struct types go up by 2 from here (even)

    };

    // All pointers are alaised and owned outside of the
    Token&              token;
    string&             parseTreeLocation;
    long                symbolType;
    FunctionPrototype*  funcType;
    StructureType*      structType;

    bool operator < (const Symbol& other) const
    {
        if (token.tokenString < other.token.tokenString)  return true;
        if (parseTreeLocation < other.parseTreeLocation) return true;

        return false;
    }

    bool operator == (const Symbol& other) const
    {
        return (token.tokenString == other.token.tokenString) &&
            (parseTreeLocation == other.parseTreeLocation);
    }

    bool tokenMatches(const Symbol& other) const
    {
        return (token.tokenString == other.token.tokenString);
    }

    bool matchSymbolAndLocation(const Symbol& other) const
    {
        // tokenString must be the same and parseTreeLocation must match the start of other.parseTreeLocation
        // Matching the start finds the definition form a potentially higher block
        return (token.tokenString == other.token.tokenString) &&
            0 == other.parseTreeLocation.find(parseTreeLocation, 0);
    }

    Symbol& operator = (const Symbol& other)
    {
        token = other.token;
        parseTreeLocation = other.parseTreeLocation;
        symbolType = other.symbolType;
        funcType = other.funcType;
        structType = other.structType;

        return *this;
    }

    Symbol(Token& inToken, string& inParseTreeLocation) :
        token(inToken),
        parseTreeLocation(inParseTreeLocation),
        symbolType(0),
        funcType(NULL),
        structType(NULL)
    {}
};

class SymbolTable
{
protected:
    // NOTE: SymbolTable creates and owns all the Symbol* in symbols.  They are all cleaned up in SymbolTable's destructor
    set<Symbol>     symbols;
    list<string*>   stringsToCleanup;

public:
    Symbol findTokenDefinition(Symbol toFind);

    Symbol findTokenDefinition(Token& token, string& location)
    {
        Symbol toFind(token, location);

        return findTokenDefinition(toFind);
    }

    long findTokenType(Token& token, string& location)
    {
        Symbol toFind(token, location);

        return findTokenDefinition(toFind).symbolType;
    }

    Symbol addOrMatchSymbol(Token& token, string& location);

    ~SymbolTable()
    {
        auto pos = stringsToCleanup.begin();
        while (pos != stringsToCleanup.end())
        {
            delete*pos;
            pos++;
        }

        stringsToCleanup.empty();
        symbols.empty();
    }
};
