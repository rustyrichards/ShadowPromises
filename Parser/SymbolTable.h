#pragma once

#include "pch.h"

/*
* This symbol table stores and matches identifiers.
*/

class FunctionPrototype;
class StructureType;

class SymbolLocation : vector<int>
{
    SymbolLocation(const vector<int>& other) : vector<int>(other)
    {
    }

    SymbolLocation(const SymbolLocation& other) : vector<int>(other)
    {
    }

    SymbolLocation(int amount) : vector<int>(amount)
    {
    }

    auto operator <=> (const SymbolLocation& other) const
    {
        int order = 0;

        int lenSelf = size();
        int lenOther = other.size();

        for (int i = 0; 0 == order && i < lenSelf && i < lenOther; i++)
        {
            order = at(i) - other.at(i);
            if (order != 0) return (0 > order) ? strong_ordering::less : strong_ordering::greater;
        }

        if (lenSelf == lenOther) strong_ordering::equal;

        return (lenSelf < lenOther) ? strong_ordering::less : strong_ordering::greater;
    }

    bool startsWith(const SymbolLocation& other) const
    {
        bool startsWithOther = true;

        int lenSelf = size();
        int lenOther = other.size();

        for (int i = 0; startsWithOther && i < lenSelf && i < lenOther; i++)
        {
            startsWithOther = at(i) == other.at(i);
        }

        if (startsWithOther) startsWithOther = lenOther <= lenSelf;

        return startsWithOther;
    }
};

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

        maxGeneralTypes = 7, 

        // Specific types
        int_type,
        float_type,
        funct_type,     // The function types go up by specificTypeAdvancement
        struct_type,    // The struct types go up by specificTypeAdvancement

        specificTypeAdvancement = 16,
    };

    // All pointers and references are alaised and owned outside of Symbol
    Token&              token;
    vector<int>&        pareseLocation;
    long                symbolType;
    FunctionPrototype*  funcType;
    StructureType*      structType;

    bool operator < (const Symbol& other) const
    {
        if (token.tokenString < other.token.tokenString)  return true;
        if (pareseLocation < other.pareseLocation) return true;

        return false;
    }

    bool operator == (const Symbol& other) const
    {
        return (token.tokenString == other.token.tokenString) &&
            (pareseLocation == other.pareseLocation);
    }

    bool tokenMatches(const Symbol& other) const
    {
        return (token.tokenString == other.token.tokenString);
    }

    bool parseLoactionStartsWith(vector<int>& otherPareseLocation) const
    {
        bool matches = true;
        if (pareseLocation.size() >= otherPareseLocation.size())
        {
            for (int i = 0; matches && i< otherPareseLocation.size(); i++)
            {
                matches = pareseLocation[i] == otherPareseLocation[i];
            }
        }
    }

    bool matchSymbolAndLocation(const Symbol& other) const
    {
        // tokenString must be the same and pareseLocation must match the start of other.pareseLocation
        // Matching the start finds the definition form a potentially higher block
        return (token.tokenString == other.token.tokenString) &&
            parseLoactionStartsWith(other.pareseLocation);
    }

    Symbol& operator = (const Symbol& other)
    {
        token = other.token;
        pareseLocation = other.pareseLocation;
        symbolType = other.symbolType;
        funcType = other.funcType;
        structType = other.structType;

        return *this;
    }

    Symbol(Token& inToken, vector<int>& inParseTreeLocation) :
        token(inToken),
        pareseLocation(inParseTreeLocation),
        symbolType(0),
        funcType(NULL),
        structType(NULL)
    {}
};

class SymbolLocationStorage
{
protected:
    const long              offsetMask = 3;
    const long              offset = 4;
    SymbolLocation          currentLocation;
    list<SymbolLocation*>   savedLocations;
    bool                    locationChanged;
public:
    enum SymbolBlockKind {
        root,
        block,
        parameters,
        prototype,
    };

    SymbolLocationStorage() :
        currentLocation(1000),
        savedLocations()
    {
        locationChanged = true;
        currentLocation.push_back(0);
    }

    ~SymbolLocationStorage()
    {
        for (auto item : savedLocations)
        {
            delete item;
        }
        savedLocations.clear();
    }


    SymbolBlockKind CurrentBlockType()
    {
        return SymbolBlockKind(currentLocation.back() & offsetMask);
    }

    void PushBlock(SymbolBlockKind type)
    {
        locationChanged = true;
        currentLocation.push_back(int(type));
    }

    void NextBlock(SymbolBlockKind type)
    {
        locationChanged = true;
        currentLocation.back() = ((currentLocation.back() + offset) & ~offsetMask) + int(type);
    }

    void PopBlock()
    {
        locationChanged = true;
        if (0 < currentLocation.size()) currentLocation.pop_back();
    }

    vector<int>& GetLocationRef()
    {
        vector<int>* locationPtr;
        if (locationChanged)
        {
            locationPtr = new SymbolLocation(currentLocation);
            savedLocations.push_back(locationPtr);
        }
        else
        {
            locationPtr = savedLocations.back();
        }
        locationChanged = false;

        return *locationPtr;
    }
};

class SymbolTable
{
protected:
    /// <summary>
    /// symbolLocation - scheme.
    ///     { 0 } is the root
    ///     Each new block depth adds a new element.
    ///     Each sibbling block takes the laste element N  -> (N + offset) & ~offest  + SymbolBlockKind
    ///     { 0, 1 } is the first block off the root
    ///     { 0, 4+3 } is a following parameter block
    ///     { 0, 8+1 } is the next block
    ///     { 0, 8+1, 1 } is the is a child block
    ///
    /// This scheme is to match the symbol in the map.  Just match the symbol by name and start of the span/array
    ///     /// </summary>
    vector<int>         symbolLocation;
    // NOTE: SymbolTable creates and owns all the Symbol* in symbols.  They are all cleaned up in SymbolTable's destructor
    set<Symbol>         symbols;
    list<vector<long>*> locationsToCleanup;

public:
    enum SymbolBlockKind {
        root,
        block,
        parameters,
        prototype,

        offset = 4,
    };

    Symbol findTokenDefinition(Symbol toFind);

    Symbol findTokenDefinition(Token& token, vector<int>& location)
    {
        Symbol toFind(token, location);

        return findTokenDefinition(toFind);
    }

    Symbol findTokenDefinition(Token& token)
    {
        return findTokenDefinition(token, symbolLocation);
    }

    long findTokenType(Token& token, vector<int>& location)
    {
        return findTokenDefinition(token, location).symbolType;
    }

    long findTokenType(Token& token)
    {
        return findTokenDefinition(token).symbolType;
    }

    Symbol addOrMatchSymbol(Token& token, vector<int>& inParseTreeLocation);

    ~SymbolTable()
    {
        auto pos = locationsToCleanup.begin();
        while (pos != locationsToCleanup.end())
        {
            delete*pos;
            pos++;
        }

        locationsToCleanup.empty();
        symbols.empty();
    }
};
