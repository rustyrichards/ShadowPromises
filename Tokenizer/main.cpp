#include "pch.h"

#include <iostream>
#include <string>
#include <fstream>      // ifstream

#include "..\Parser\ShadowPromisesTokenizer.h"

int main(int argc, char* argv[], char* envp[])
{
    Tokenizer& shadowPromisesTokenizer = initShadowPromisesTokenizer();

    bool wasInputFileFound = false;


    if (argc > 1)
    {
        ifstream fileInput;

        char* option = NULL;
        for (int i = 1; i < argc; i++)
        {
            if (fileInput.is_open()) fileInput.close();

            if (L'-' == argv[i][0])
            {
                option = argv[i] + 1;
            }
            else
            {
                try
                {
                    wasInputFileFound = true;
                    boost::filesystem::path testPath(argv[i]);
                    std::cout << "Tokenizing \"" << argv[i] << "\"" << endl << endl;

                    shadowPromisesTokenizer.tokenize(testPath);

                    dumpTokens(std::cout, shadowPromisesTokenizer.tokens);

                }
                catch (exception& ex)
                {
                    std::cout << "Could not open \"" << argv[i] << "\" as a file.  " <<
                        ex.what() << endl << endl;
                }

                shadowPromisesTokenizer.cleanup();
            }
        }
    }

    if (!wasInputFileFound)
    {
        std::cout << "Enter the text to tokenize:" << endl;

        shadowPromisesTokenizer.tokenize(std::cin);
        dumpTokens(std::cout, shadowPromisesTokenizer.tokens);

        shadowPromisesTokenizer.cleanup();
    }
}
