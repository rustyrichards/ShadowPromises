#ifndef SHADOW_PROMISES_TOKENIZER_H_INCLUDED
#define SHADOW_PROMISES_TOKENIZER_H_INCLUDED

#include "Tokenizer.h"

extern "C" EXPORT Tokenizer & initShadowPromisesTokenizer();

extern "C" EXPORT void dumpTokens(
    std::ostream& output, 
    std::vector<Token> tokens);

extern "C" EXPORT void handleCommandlineAndTokenize(
    Tokenizer& shadowPromisesTokenizer,
    std::istream& initialInput,
    std::ostream& output,
    const char* inputPrompt = NULL,
    int argc = 0,
    char* argv[] = NULL,
    char* envp[] = NULL);

#endif SHADOW_PROMISES_TOKENIZER_H_INCLUDED