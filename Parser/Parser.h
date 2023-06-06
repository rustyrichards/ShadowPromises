#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "pch.h"

/*
* The Goal of Parser is to turn the tokens into valid statements, or reasonable error messages.
* It should:
*   1) Match the tokens to valid statements
*       a) Valid statementes are added to the parse tree
*       b) Invalid statements are formattted into an error for display
*   2) Set variable types
*       a) The variable type is set once.  The symbol table is used to match a token to its typed variable
*       b) Variable type mismatches are formatted into an error for display
*   3) The parse rules are based on state and matching tokens. These rules can be replaced
*/

enum ParseStates {
    None = 0,
    Root = 1,           // Outside of any block.
    InBlock,
    InPrototype,
    InParameters,


    // Flags
    InLoop = 16,        // Inside of a loop (Loop exits allowed.)
    InFunction = 32,    // Inside of a functions.
};

// The parse was becomming too complicated.
// Instead use the same syntax that the tokenizer reads with these extensions:
/*
* | is optional
* ! is a state change
*   !enterLoop
*   !exitLoop
*   !enterFunc
*   !exitFunc
*   See the ShadowPromisesTokenizer.cpp {}, {}, and () already set state enter and exit
* # - resolves to value
*   #bool
*   #int    - Note int will auto convert to float if neded (some percision may be lost with very large 64 bit integers)
*             float will not auto convert to int.  The fraction after the decimal is always lost in converting to int.
*   #float
*   #string
*   #type   - any type
*   #funct  - any function
*
* In ShadowPromises the if statement is:
*   :test #bool :if { #statements } | :else { #statements }
*
* In ShadowPromises the loop statement is:
*   :loop { !enterLoop #statements  !exitLoop }
*
* In ShadowPromises a function definition is:
*   [ #prototypes ] { !enterFunc #statements  !exitFunction }
*/
#endif // PARSER_H_INCLUDED
