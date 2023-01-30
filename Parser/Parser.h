#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "framework.h"
#include "Tokenizer.h"

using namespace std;

class EXPORT funct
{
};

class EXPORT ParseNode
{
protected:
	void decodeExponentialToDouble(string_view numberString);
	void decodeHexToDouble(string_view numberString);
	void decodeString(string_view maybeEscaped);
	bool setEscapeValue(char*& dest, const char*& runner, const char* end);

public:
	enum nodeType {
		integerValue = 1,
		doubleValue,
		stringValue,
		structValue,

		functon,

		value = 1024,	// less than value means it is a value type

		statement,
        ifElse,
        loop
	};

	union nodeValue {
		nodeValue(double inVal) : doubleVal(inVal) {};
		nodeValue(long inVal) : longVal(inVal) {};
		nodeValue(string_view inVal) : stringVal(inVal) {};

		long		longVal;
		double		doubleVal;
		string_view stringVal;
		funct		functValue;
	};

	long		nodeType;
	nodeValue	val;
	char*		replacementStringBuffer;
	Token&		token;		// Note: token.typeAndFlags.type will be adjusted to the type that fits the token and parsing state.
	ParseNode*	next;

	ParseNode* identifier;	// For nodes that need an identifier
	ParseNode* parameters;	// For nodes that have a parameter list.  May be a parameter list variable
	ParseNode* block;		// For a new block

	bool isFunction;		// true for a function definition

	std::string parsingError;	// Nun-empty if there was an error

	inline long getType() { return (unsigned long)token.typeAndFlags.type; }
	inline long getParsingFlags() { return (unsigned long)token.typeAndFlags.parsingFlags; }
	void setType(Token::TokenType inType);

	inline bool hasParsingFlags(long flags)
	{
		return token.hasParsingFlags(flags);
	}

	inline bool hasRequirementsFlags()
	{
		return  token.hasRequirementsFlags();
	}

	inline bool hasFollowsFlags()
	{
		return  token.hasFollowsFlags();
	}

	inline bool hasNoError()
	{
		return 0 == parsingError.length();
	}

	ParseNode(Token& inToken, Token::TokenType inType = Token::unknownToken);

	~ParseNode()
	{
		delete next;
		delete identifier;
		delete parameters;
		delete block;
		delete[] replacementStringBuffer;
	}
};


class EXPORT Parser
{
public:
	enum {
		top,
		prototype,
		functionBody,
		callParameters,
		logicalTest,		// Like function parameters, the the evaluation shortcuts

		nonBitFlagMask = Token::testResult - 1,

		// Bit flags
		testResultAvailable = Token::testResult,
		elseIsAllowed = Token::testResult << 1,
		inBlock = Token::testResult << 2,
		loopExitAllowed = Token::testResult << 3,
	};

protected:
	static Token& genericBlock;

	long parseFunctionCall(
		long nextType,
		ParseNode* current,
		long depth,
		long state);

	long parseBlock(
		long nextType,
		ParseNode* current,
		long depth,
		long state);

	ParseNode* internalParseParameterList(
		long depth,
		long state);

	ParseNode* internalParse(
		long depth = 0,
		long state = top);

	token_vector::iterator pos;
	token_vector::iterator end;

	Tokenizer& tokenizer;

public:
	inline long parseStateWihoutFlags(long currentState)
	{
		return currentState & nonBitFlagMask;
	}
	inline long matchParseState(long currentState, long desiredState)
	{
		return (currentState & nonBitFlagMask) == (desiredState & nonBitFlagMask);
	}
	inline long changeParseState(long currentState, long newState)
	{
		// The current bit falgs with the non bit mask flags of newState
		return (currentState & ~nonBitFlagMask) | (newState & nonBitFlagMask);
	}

	inline long setParsingFlag(long currentState, long newState)
	{
		// OR in the flags from newState
		return currentState | (newState & ~nonBitFlagMask);
	}

	inline long clearParsingFlag(long currentState, long newState)
	{
		// AND the bit inversion of  the flags from newState  (Remove the selected flags)
		return currentState & (~(newState & ~nonBitFlagMask));
	}

	inline long setTestResultAndElseAllowedFromTokenFlags(long currentState, long tokenFlags)
	{
		// NOTE: testResultAvailable == Token::testResult
		//       and elseIsAllowed == Token::elseAllowed == Token::testResult << 1
		return (currentState & ~(testResultAvailable| elseIsAllowed)) |
			(tokenFlags & (testResultAvailable | elseIsAllowed));
	}

	Parser(map<char, TokenMatching*>* matchPatterns) : tokenizer(*new Tokenizer(matchPatterns)) {}
	Parser(Tokenizer& inTokenizer) : tokenizer(inTokenizer) {}

	inline void tokenize(istream& input) { tokenizer.tokenize(input); }
	inline void tokenize(boost::filesystem::path& filePath) { tokenizer.tokenize(filePath); }
	inline void tokenize(string_view stringBuffer) { tokenizer.tokenize(stringBuffer); }

	inline ParseNode* parseTokens()
	{
		pos = tokenizer.tokens.begin();
		end = tokenizer.tokens.end();

		return internalParse();
	}

	inline ParseNode* parse(istream& input) 
	{ 
		tokenizer.tokenize(input);
		return parseTokens();
	}
	inline ParseNode* parse(boost::filesystem::path& filePath)
	{
		tokenizer.tokenize(filePath);
		return parseTokens();
	}
	inline ParseNode* parse(string_view stringBuffer)
	{
		tokenizer.tokenize(stringBuffer);
		return parseTokens();
	}
};

#endif // PARSER_H_INCLUDED
