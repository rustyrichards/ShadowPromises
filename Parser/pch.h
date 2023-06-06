// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <string>
#include <xstring>
#include <array>
#include <format>
#include <iosfwd>
#include <iostream>
#include <map>
#include <list>
#include <set>
#include <span>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include "interop.h"
#include "ReadFileData.h"
#include "framework.h"
#include "TokenScanning.h"
#include "Tokenizer.h"
#include "SymbolTable.h"
#include "Parser.h"
#include "ShadowPromisesTokenizer.h"

// The C++ 20 STL COOKBOOK  - in general it seems to be a good book
// Claimied that it is a better practace to just using for the specific
// classes like:  using namespace std::string
// This may matter for Modules.  For old style #include you only get
// the headers you actually included.
//
// That does not work.  using namespace std::string, using namespace std::basic_strig - doesn't work.
using namespace std;
#endif //PCH_H
