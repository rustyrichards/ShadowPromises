#include "pch.h"

ParseNode::ParseNode(Token& inToken, Token::TokenType inType) :
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

Token& Parser::genericBlock = *new Token(0, 0, Token::block_start, Token::blockFollows);

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
		if (NULL == head) head = newNode;
		if (NULL != current) current->next = newNode;
		current = newNode;

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

		if (current->hasParsingFlags(Token::parametersFollows))
		{
			if (Token::params_start == nextType)
			{
				pos++;		// Jump over the "(" we are making the parameter list; we no-longer need the token
				current->parameters = internalParse(depth + 1, 
					// NOTE: prototype and parameters do not count as being in a block!
					clearParsingFlag(changeParseState(state, callParameters), inBlock));

				// Pos will have advanced - but 2 parameter lists in a row is not allowed, so just advance next type and continue.
				nextType = pos->typeAndFlags.type;
			}
			else if (Token::identifier == nextType)
			{
				// TODO: needs to validate that identifier is a parameter list
				current->parameters = new ParseNode(*pos++, Token::identifier);
			}
			else
			{
				current->parsingError = "Error:  This token requires a paramter list - " + current->token.errorDisplay();
			}
		}

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
			if (Token::identifier == type)
			{
				// TODO: needs to validate that identifier is a function
				pos++;		// Jump over the "(" we are making the parameter list; we no-longer need the token
				current->parameters = internalParse(depth+1, 
					// NOTE: prototype and parameters do not count as being in a block!
					clearParsingFlag(changeParseState(state, callParameters), inBlock));
			}
			else
			{
				current->parsingError = "Error:  This token should not be followed by parameters - " + current->token.errorDisplay();
			}
		}

		if (Token::block_start == nextType)
		{
			if (!current->hasParsingFlags(Token::blockFollows))
			{
				// Not part of the statement like if, else or loop  here just generate a parent for the block
				newNode = new ParseNode(genericBlock);
				current->next = newNode;
				current = newNode;
			}

			// isLoopExitAllowed | 0 != flags & Token::loopBlock - need to be able to exit from within an if or nested block in a loopBlock
			pos++;		// Jump over the "{" we are making the block we no-longer need the token
			
			long blockState = setParsingFlag(state, inBlock);
			if (current->isFunction) blockState = changeParseState(blockState, functionBody);
			if (0 != (flags & Token::loopBlock)) blockState = setParsingFlag(blockState, loopExitAllowed);
			current->block = internalParse(depth+1, blockState);
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
