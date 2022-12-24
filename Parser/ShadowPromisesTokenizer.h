#ifndef SHADOW_PROMISES_TOKENIZER_H_INCLUDED
#define SHADOW_PROMISES_TOKENIZER_H_INCLUDED

#include "Tokenizer.h"

extern "C++" EXPORT Tokenizer& initShadowPromisesTokenizer();

extern "C" EXPORT void dumpTokens(
    std::ostream& output, 
    std::vector<Token> tokens);

#endif //SHADOW_PROMISES_TOKENIZER_H_INCLUDED