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
* Optional
*   Parse:opt   - the remainer is optional
* 
* State change
*   Parse:enterLoop
*   Parse:exitLoop
*   Parse:enterFunc
*   Parse:exitFunc
*   See the ShadowPromisesTokenizer.cpp {}, {}, and () already set state enter and exit
* 
* Values
*   Parse:bool
*   Parse:int    - Note int will auto convert to float if neded (some percision may be lost with very large 64 bit integers)
*                  float will not auto convert to int.  The fraction after the decimal is always lost in converting to int.
*   Parse:float
*   Parse:string
*   Parse:type   - any type
*   Parse:funct  - any function
*
* In ShadowPromises the if statement is:
*   :test Parse:bool :if { #statements } Parse:opt :else { #statements }
*
* In ShadowPromises the loop statement is:
*   :loop { Parse:enterLoop #statements  Parse:exitLoop }
*
* In ShadowPromises a function definition is:
*   [ #prototypes ] { Parse:enterFunc #statements  Parse:exitFunction }
*/
#endif // PARSER_H_INCLUDED
