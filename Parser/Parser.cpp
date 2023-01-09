#include "pch.h"
#include <cmath>
#include <format>
#include <limits>

Token& Parser::genericBlock = *new Token(0, 0, Token::block_start, Token::blockFollows);

unsigned long decodeHex(
	const char*& runner,
	const char* end,
	char* dest = NULL,	//  NULL means just make the unsigned long
	bool isUtf = false,
	bool braceEnds = false
)
{
	unsigned long result = 0;

	const char* start = runner;

	while (runner < end)
	{
		if ('0' <= *runner && *runner <= '9')
		{
			result *= 16;
			result += *runner - '0';
		}
		else if ('a' <= *runner && *runner <= 'f')
		{
			result *= 16;
			result += 10 + (*runner - 'a');
		}
		else if ('A' <= *runner && *runner <= 'F')
		{
			result *= 16;
			result += 10 + (*runner - 'A');
		}
		else if (braceEnds && ('}' == *runner))
		{
			runner++;
			break;
		}
		runner++;

		// If we have a buffer and it is not a utf code point we should output each byte
		// If runner-start is even we have read 2 hex digits for 1 byte
		if (!isUtf && (NULL != dest) && !((runner - start) & 1))
		{
			*(dest++) = result & 0x0ff;
		}
	}

	if (isUtf && (NULL != dest))
	{
		// Codepoint to utf8
		// 
		// Output result as utf8
		if (0x07fL >= result)
		{
			*(dest++) = result & 0x07f;
		}
		else if (0x07ffL >= result)
		{
			*(dest++) = 0x0c0 + (result >> 6);
			*(dest++) = 0x080 + (result & 0x03f);
		}
		else if (0x0ffffL >= result)
		{
			*(dest++) = 0x0E0 + (result >> 12);
			*(dest++) = 0x080 + ((result >> 6) & 0x03f);
			*(dest++) = 0x080 + (result & 0x03f);
		}
		else if (0x01FFFFL >= result)
		{
			*(dest++) = 0x0E0 + (result >> 18);
			*(dest++) = 0x080 + ((result >> 12) & 0x03f);
			*(dest++) = 0x080 + ((result >> 6) & 0x03f);
			*(dest++) = 0x080 + (result & 0x03f);
		}
	}

	return result;
}

void ParseNode::decodeHexToDouble(string_view numberString)
{
	nodeType = ParseNode::doubleValue;

	const char* runner = numberString.data();
	runner += 2;	// jump over 0x

	const char* end = runner + numberString.size();

	delete[] replacementStringBuffer;
	// no need to free the scalars.
	replacementStringBuffer = 0;

	val.doubleVal = (double)decodeHex(runner, end);
}

long long internalDecodeIntegralPart(string_view::const_iterator& runner, string_view::const_iterator& end, long& digitsAfterDecimal)
{
    // Simplification trick  start digitsAfterDecimal very negative that way it will stay negative for any enterable number of digits.
    // (You can't enter bilions of digits.)
    // Only encountering '.' will cange it to 0 and start it moving to positive.
    digitsAfterDecimal = LONG_MIN;

    long long integralPart = 0;      // Keep out roundoff errors atleast until the exponent

    bool isNegative = false;

    while (runner != end)
    {
        if ('-' == *runner)
        {
            isNegative = !isNegative;	// Incase multiple negatives are allowed.
        }
        else if ('0' <= *runner && *runner <= '9')
        {
            integralPart *= 10L;
            integralPart += *runner - '0';
            digitsAfterDecimal++;
        }
        else if ('.' == *runner)
        {
            digitsAfterDecimal = 0;
        }
        else
        {
            break;
        }
        runner++;
    }

    // If the ',' was not found.  We have no digits after the decimal
    if (0L > digitsAfterDecimal) digitsAfterDecimal = 0;

    // Negate the value if an odd number of '-' were found.
    return (isNegative) ? -integralPart : integralPart;
}

double internalDecodeExponentialToDouble(string_view::const_iterator& runner, string_view::const_iterator& end)
{
	double val = NAN;

    long digitsAfterDecimal = 0;

    // Keep out roundoff errors atleast until the exponent
    long long integralPart = internalDecodeIntegralPart(runner, end, digitsAfterDecimal);

    long long base10Exponent = -digitsAfterDecimal;

	if (runner != end &&  ('e' == *runner || 'E' == *runner))
	{
        base10Exponent += internalDecodeIntegralPart(++runner, end, digitsAfterDecimal);
	}

    if (0 != base10Exponent)
    {
        val = (double)integralPart * pow(10.0, base10Exponent);
    }
    else
    {
        val = integralPart;
    }

    return val;
}


void ParseNode::decodeExponentialToDouble(string_view numberString)
{
	nodeType = ParseNode::doubleValue;

	delete[] replacementStringBuffer;
	// no need to free the scalars.
	replacementStringBuffer = NULL;

	string_view::const_iterator runner = numberString.begin();
	string_view::const_iterator end = numberString.end();
	val.doubleVal = internalDecodeExponentialToDouble(runner, end);
}

bool ParseNode::setEscapeValue(char*& dest, const char*& runner, const char* end)
{
	bool success = true;

	bool isUtf = false;
	switch (*runner)
	{
	case '\'' :
	case '"' :
	case '?' :
	case '\\' :
		*(dest++) = *runner;
		break;
	case 'a' :
		*(dest++) = 0x07;	// audible bell
		break;
	case 'b' :
		*(dest++) = 0x08;	// backspace
		break;
	case 'f' :
		*(dest++) = 0x0c;	// form feed
		break;
	case 'n' :
		*(dest++) = 0x0a;	// new line
		break;
	case 'r' :
		*(dest++) = 0x0d;	// carriage return
		break;
	case 't' :
		*(dest++) = 0x09;	// tab
		break;
	case 'v' :
		*(dest++) = 0x0b;	// vertical tab
		break;
	case 'u' :
		isUtf = true;
		// Fallthrought wanted.
	case 'x' :
		if (runner + 3 < end && '{' == *(runner + 1))
		{
			runner += 2;	// Jump the "x{" or "u{" to the number start
			decodeHex(
				runner,
				end,
				dest,
				isUtf,
				true
			);

			success = getType() < Token::failures;
		}
		else
		{
			success = false;
			setType(Token::badString);
			parsingError = std::format("Error:  Bad character sequence \\{} at: {}", *runner, token.startingCharacter + (replacementStringBuffer - runner));
		}
		break;
	default:
		success = false;
		parsingError = std::format("Error:  Bad character sequence \\{} at: {}", *runner, token.startingCharacter + (replacementStringBuffer - runner));
	}

	return success;
}

void ParseNode::decodeString(string_view maybeEscaped)
{
	nodeType = ParseNode::stringValue;
	val.stringVal = maybeEscaped;

	auto pos = token.tokenString.find('\\');

	delete[] replacementStringBuffer;
	replacementStringBuffer = NULL;

	if (0 <= pos)
	{
		// '\\' found - we need to decode
		long len = token.tokenString.size();
		replacementStringBuffer = new char[len];
		token.tokenString.copy(replacementStringBuffer, len);

		char* dest = replacementStringBuffer;
		const char* runner = dest;
		const char* end = runner + len;

		while (runner < end)
		{
			if ('\\' == *runner)
			{
				// a '\\' at the every end is just a '\\'
				if (++runner < end)
				{
					if (!setEscapeValue(dest, runner, end))
					{
						// failure of setEscapeValue will set badString
						delete[] replacementStringBuffer;	// do not keep the bad replacementStringBuffer
						replacementStringBuffer = NULL;
						break;	
					}
				}
			}
			else if (dest != runner)
			{
				*(dest++) = *(runner++);
			}
			else
			{
				dest++;
				runner++;
			}
		}

		// If we had to allocate a string, set the val.stringVal according to the used string buffer.
		if (NULL != replacementStringBuffer)
		{
			val.stringVal = string_view(replacementStringBuffer, dest - replacementStringBuffer);
		}
	}
}

void ParseNode::setType(Token::TokenType inType)
{
	
	// Do not change token.typeAndFlags.type if it is already marked as a failure
	if (Token::failures > token.typeAndFlags.type)
	{
		// Do not change to Token::unknownToken or anything less
		if (Token::unknownToken < inType) token.typeAndFlags.type = inType;

		switch (token.typeAndFlags.type)
		{
		case Token::number:
			decodeExponentialToDouble(token.tokenString);
			break;
		case Token::hexNumber:
			decodeHexToDouble(token.tokenString);
			break;
		case Token::string:
			decodeString(token.tokenString);
			break;
		// NOTE:  the function data is not read in.  It will be assidned later.
		default:
			nodeType = ParseNode::statement;
			break;
		}
	}
}

ParseNode::ParseNode(Token& inToken, Token::TokenType inType) :
	nodeType(ParseNode::statement),
	val(NAN),
	replacementStringBuffer(NULL),
	token(inToken),
	next(),
	identifier(),
	parameters(),
	block(),
	isFunction(false),
	parsingError(""sv)
{
	setType(inType);
}

inline void adjustCurrentAndHeadNode(ParseNode*& head, ParseNode*& current, ParseNode* newNode)
{
	if (NULL == newNode) return;

	if (NULL == head) head = newNode;
	if (NULL != current) current->next = newNode;
	current = newNode;
}

long Parser::parseFunctionCall(
	long nextType,
	ParseNode* current,
	long depth,
	long state)
{
	if (NULL != current)
	{
		// A logical operation (and, or, nand), or other function like keyword, or an function call
		if (current->hasParsingFlags(Token::parametersFollows) || Token::identifier == current->getType())
		{
			// TODO:  if Token::identifier == type need to validate that it is a function
			pos++;		// Jump over the "(" we are making the parameter list; we no-longer need the token
			current->parameters = internalParse(depth + 1,
				// NOTE: prototype and parameters do not count as being in a block!
				clearParsingFlag(changeParseState(state, callParameters), inBlock));

			// Pos will have advanced - but 2 parameter lists in a row is not allowed, so just advance next type and continue.
			nextType = pos->typeAndFlags.type;
		}
		else
		{
			current->parsingError = "Error:  This token is not a valid function in a parameter list  - " + current->token.errorDisplay();
		}
	}

	return nextType;
}

long Parser::parseBlock(
	long nextType,
	ParseNode* current,
	long depth,
	long state)
{
	if (!current->hasParsingFlags(Token::blockFollows))
	{
		// Not part of the statement like if, else or loop  here just generate a parent for the block
		auto newNode = new ParseNode(genericBlock);
		current->next = newNode;
		current = newNode;
	}

	// isLoopExitAllowed | 0 != flags & Token::loopBlock - need to be able to exit from within an if or nested block in a loopBlock
	pos++;		// Jump over the "{" we are making the block we no-longer need the token

	long blockState = setParsingFlag(state, inBlock);
	if (current->isFunction) blockState = changeParseState(blockState, functionBody);
	if (0 != (current->getParsingFlags() & Token::loopBlock)) blockState = setParsingFlag(blockState, loopExitAllowed);

	// NOTE: use the general internalParse.  The only difference from the root is that a function block supports :self and :return
	current->block = internalParse(depth + 1, blockState);

	return pos->typeAndFlags.type;
}

ParseNode* Parser::internalParseParameterList(
	long depth,
	long state)
{
	ParseNode* head = NULL;
	ParseNode* current = NULL;

	while (pos != end)
	{
		ParseNode* newNode = new ParseNode(*pos++);

		long type = newNode->getType();
		switch (type)
		{
		case Token::params_end:
			delete newNode;	// Do not save the close node
			return head;
			break;
		case Token::block_start:	// Blocks not allowe in a parameter list!
			newNode->parsingError = "Error:  A block cannot start in a parameter list - " + newNode->token.errorDisplay();
			break;
		case Token::block_end:
		case Token::params_start:	// Not allowed in the middle of a parameter list - only allowed after a function - that will be picked up below
		case Token::prototype_start:
		case Token::prototype_end:
		case Token::endOfInput:
				newNode->parsingError = "Error:  This token is not allowed in a parameter list - " + newNode->token.errorDisplay();
		case Token::keyword:
		case Token::functionReturn:
		case Token::selfCall:
			// Few keywords are allowed in parameter lists
			if (!current->hasParsingFlags(Token::allowedInParameters))
			{
				newNode->parsingError = "Error:  This keyword is not allowed in a parameter list - " + newNode->token.errorDisplay();
			}
			break;
		}

		// If we haven't exited this sub-tree connect current and head 
		adjustCurrentAndHeadNode(head, current, newNode);

		if (NULL != current && current->hasNoError())
		{
			auto flags = current->getParsingFlags();

			// The rejected keywords, functionReturn and selfCall mean it is not necessary to test the rest of the parsing flags here

			auto nextType = pos->typeAndFlags.type;

			if (Token::params_start == nextType)
			{
				nextType = parseFunctionCall(nextType, current, depth, state);
			}
		}

		// Do not need to check other current->hasParsingFlags current->hasParsingFlags(Token::allowedInParameters) will have covered that.

		// :test , :if , :else are not allowed here so no need to setTestResultAndElseAllowedFromTokenFlags
	}

	// End of input;
	if (0 != depth && NULL != current)
	{
		current->parsingError = "Error:  Unexpected end of input - " + current->token.errorDisplay();
	}
	return head;
}

ParseNode* Parser::internalParse(
	long depth,
	long state)
{
	ParseNode* head = NULL;
	ParseNode* current = NULL;

	while (pos != end)
	{
		ParseNode* newNode = new ParseNode(*pos++);

		// Exit if we are done with input, a block, or parameters
		switch (newNode->getType())
		{
		case Token::params_end:
			if (matchParseState(state, callParameters))
			{
				return head;
			}
			else
			{
				newNode->parsingError = "Error:  This token must end a parameter list - " + newNode->token.errorDisplay();
			}
			break;
		case Token::prototype_end:
			if (matchParseState(state, prototype))
			{
				return head;
			}
			else
			{
				newNode->parsingError = "Error:  This token must end a function prototype - " + newNode->token.errorDisplay();
			}
			break;
		case Token::block_end:
			if (state & inBlock)
			{
				return head;
			}
			else
			{
				newNode->parsingError = "Error:  This token must end a block - " + newNode->token.errorDisplay();
			}
			break;
		case Token::endOfInput:
			if (0 != depth)
			{
				// If we ended in error.  Add the EOF token to the current parse as the next.
				if (NULL != current) current->next = newNode;
				newNode->parsingError = "Error:  Unexpected end of input - " + newNode->token.errorDisplay();
			}
			--pos;
			return head;
		}

		// If we haven't exited this sub-tree connect current and head 
		adjustCurrentAndHeadNode(head, current, newNode);

		// Handle making the function start from the start of the < ...> prototype
		if (Token::prototype_start == current->getType())
		{
			// Remove the "<" and make sure the node is flagged as a function.
			current->isFunction = true;
			current->token.tokenString = ""sv;

			// read the prototype into parameters
			current->parameters = internalParse(depth + 1, 
				// NOTE: prototype and parameters do not count as being in a block!
				clearParsingFlag(changeParseState(state, prototype), inBlock));
		}

		auto flags = current->getParsingFlags();

		if (current->hasRequirementsFlags())
		{
			if (current->hasParsingFlags(Token::requiresTestResult) &&
				!(state & testResultAvailable))
			{
				current->parsingError = "Error:  There is no :test corresponding to this token - " + current->token.errorDisplay();
			} // else no problem

			if (current->hasParsingFlags(Token::requiresElseAllowed) &&
				!(state & elseIsAllowed))
			{
				current->parsingError = "Error:  There is no :if corresponding to this token - " + current->token.errorDisplay();
			} // else no problem

			if (current->hasParsingFlags(Token::requiresLoopBlock) &&
				!(state & loopExitAllowed))
			{
				current->parsingError = "Error:  There is no :loop coresponding to this token - " + current->token.errorDisplay();
			} // else no problem

			if (current->hasParsingFlags(Token::requiresFunctionBlock))
			{
				if (!matchParseState(state, functionBody))
				{
					current->parsingError = "Error:  This symbol is only allowed in a function block - " + current->token.errorDisplay();
				}
			}
		}

		auto nextType = pos->typeAndFlags.type;

		long type = current->getType();

		if (current->hasFollowsFlags())
		{
			if (current->hasParsingFlags(Token::identifierFollows))
			{
				if (Token::identifier == nextType ||
					// Assignment to return is allowed where assignment to an identifier is usually wanted.
					(Token::functionReturn == nextType && type == Token::assignment && matchParseState(state, functionBody)))
				{
					current->identifier = new ParseNode(*pos++, Token::identifier);
				}
				else
				{
					current->parsingError = "Error:  This token requires a following identifier - " + current->token.errorDisplay();
				}
			}

			if (current->hasParsingFlags(Token::nameFollows))
			{
				if (Token::identifier == nextType)
				{
					current->identifier = new ParseNode(*pos++, Token::name);
				}
				else
				{
					current->parsingError = "Error:  This token requires a following name identifier - " + current->token.errorDisplay();
				}
			}

			if (current->hasParsingFlags(Token::compileFlagFollows))
			{
				if (Token::identifier == nextType)
				{
					current->identifier = new ParseNode(*pos++, Token::compilerFlag);
				}
				else
				{
					current->parsingError = "Error:  This token requires a following compiler flag identifier - " + current->token.errorDisplay();
				}
			}

			if (current->hasParsingFlags(Token::blockFollows))
			{
				if (Token::block_start != nextType)
				{
					current->parsingError = "Error:  A block must follow this token - " + current->token.errorDisplay();
				}
			}
		}

		if (matchParseState(state, callParameters) && (Token::keyword == type) && (0 == (flags & Token::allowedInParameters)))
		{
			current->parsingError = "Error:  This keyword is not allowed in parameters - " + current->token.errorDisplay();
		}

		if (Token::params_start == nextType)
		{
			nextType = parseFunctionCall(nextType, current, depth, state);
		}

		if (Token::block_start == nextType)
		{
			nextType = parseBlock(nextType, current, depth, state);
		}

		// Here the test or if statement may have finished
		state = setTestResultAndElseAllowedFromTokenFlags(state, flags);
	}

	// End of input;
	if (0 != depth && NULL != current)
	{
		current->parsingError = "Error:  Unexpected end of input - " + current->token.errorDisplay();
	}
	return head;
}
