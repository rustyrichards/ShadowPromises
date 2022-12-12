#include "pch.h"

#include <iostream>

#include "..\Parser\ShadowPromisesTokenizer.h"

int main(int argc, char* argv[], char* envp[])
{
    Tokenizer& shadowPromisesTokenizer = initShadowPromisesTokenizer();

    handleCommandlineAndTokenize(shadowPromisesTokenizer, std::cin, std::cout, "Enter the text to tokenize:", argc, argv, envp);

    dumpTokens(std::cout, shadowPromisesTokenizer.tokens);

    shadowPromisesTokenizer.cleanup();

}
