#include "pch.h"
#include <sstream>
#include <string>
#include <iostream>
#include "CppUnitTest.h"
#include "..\Parser\ShadowPromisesTokenizer.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TokenizerTests
{
	TEST_CLASS(TokenizerTests)
	{
	private:
		Tokenizer& shadowPromisesTokenizer;
	public:
		TokenizerTests() :
			shadowPromisesTokenizer(initShadowPromisesTokenizer())
		{
			Logger::WriteMessage("In TokenizerTests()");
		}

		~TokenizerTests()
		{
			Logger::WriteMessage("In ~TokenizerTests()");
		}


		TEST_METHOD(TokenizeOneLine)
		{
			shadowPromisesTokenizer.cleanup();

			Logger::WriteMessage("In TokenizeOneLine");
			auto source = std::istringstream("5.0|var1 # Assigning to var1");

			shadowPromisesTokenizer.tokenize(source);

			auto tokenIter = shadowPromisesTokenizer.tokens.begin();
			Assert::AreEqual("5.0"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("|"sv, tokenIter->tokenString);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((int)Token::Token::assignment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)5, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("var1"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::Token::identifier, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)6, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# Assigning to var1"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::Token::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)12, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual(""sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::Token::endOfInput, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)31, tokenIter->startingCharacter);

			//// Wipe the buffer with L' '
			//char* start = shadowPromisesTokenizer.inputBuffer;
			//char* end = start + shadowPromisesTokenizer.inputBufferSize;
			//while (start < end)
			//{
			//	*(start++) = L' ';
			//}

			//// The tonken should still point into the buffer, so it will be blank too.
			//tokenIter = shadowPromisesTokenizer.tokens.begin();
			//Assert::AreEqual("   "sv, tokenIter->tokenString);
		}

		TEST_METHOD(TokenizeMultiLine)
		{
			shadowPromisesTokenizer.cleanup();

			Logger::WriteMessage("In TokenizeMultiLine");

			// ste stringstream is just like TokenizeOneLine except it uses \n as the separators
			auto source = std::istringstream("5.0\r\n|\nvar1\r\n# Assigning to var1");

			shadowPromisesTokenizer.tokenize(source);

			auto tokenIter = shadowPromisesTokenizer.tokens.begin();
			Assert::AreEqual("5.0"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("|"sv, tokenIter->tokenString);
			Assert::AreEqual((long)2, tokenIter->startingLine);
			Assert::AreEqual((int)Token::Token::assignment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("var1"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::Token::identifier, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)3, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# Assigning to var1"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::Token::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)4, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual(""sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::Token::endOfInput, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)4, tokenIter->startingLine);
			Assert::AreEqual((long)20, tokenIter->startingCharacter);
		}

		TEST_METHOD(TokenizeAllTypes)
		{
			shadowPromisesTokenizer.cleanup();

			Logger::WriteMessage("In TokenizeAllTypes");
			auto source = std::istringstream(
				":test(:equals(x y)) # :test and :if on separate lines\n"
				":if {\n"
				"  1.0 | z\n"
				"} 'simple string'\n"
				"\"Unterminated string.\n"
				":loop {\n"
				"  :test(true) :exit\n"
				"}\n"
			);

			shadowPromisesTokenizer.tokenize(source);

			auto tokenIter = shadowPromisesTokenizer.tokens.begin();
			Assert::AreEqual(":test"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("("sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::params_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":equals"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("("sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::params_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("x"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("y"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(")"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::params_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(")"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::params_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("# :test and :if on separate lines"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::comment, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":if"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("{"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::block_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("1.0"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::number, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("|"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::assignment, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("z"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("}"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::block_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("'simple string'"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::string, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("\"Unterminated string."sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::badString, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":loop"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("{"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::block_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":test"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("("sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::params_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("true"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(")"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::params_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":exit"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("}"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::block_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual((int)Token::TokenType::endOfInput, tokenIter->typeAndFlags.type);
		}

		TEST_METHOD(TokenizeMultiLineString)
		{
			shadowPromisesTokenizer.cleanup();

			Logger::WriteMessage("In TokenizeMultiLineString");

			// A valid multi line string
			shadowPromisesTokenizer.tokenize("+'Line 1 of multi-line string\n"
				"Line 2 of multi-line string' :test\n"sv
			);

			auto tokenIter = shadowPromisesTokenizer.tokens.begin();
			Assert::AreEqual("+'Line 1 of multi-line string\nLine 2 of multi-line string'"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::string, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual(":test"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::keyword, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)2, tokenIter->startingLine);
			Assert::AreEqual((long)30, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual((int)Token::TokenType::endOfInput, tokenIter->typeAndFlags.type);

			shadowPromisesTokenizer.cleanup();

			// An unterminated multi-line string
			shadowPromisesTokenizer.tokenize(
				"+'Line 1 of multi-line string\n"
				"Line 2 of multi-line string :test\n"sv
			);

			auto tokenIter2 = shadowPromisesTokenizer.tokens.begin();
			Assert::AreEqual("+'Line 1 of multi-line string\nLine 2 of multi-line string :test\n"sv, tokenIter2->tokenString);
			Assert::AreEqual((int)Token::TokenType::badString, tokenIter2->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter2->startingLine);
			Assert::AreEqual((long)1, tokenIter2->startingCharacter);

			tokenIter2++;
			Assert::AreEqual((int)Token::TokenType::endOfInput, tokenIter2->typeAndFlags.type);
		}

		TEST_METHOD(TokenizeMemoryMappedFile)
		{
			shadowPromisesTokenizer.cleanup();

			Logger::WriteMessage("In TokenizeMemoryMappedFile");

			// A file path (relative) to load as a memory mapped file.
			boost::filesystem::path testPath("TestCode.sp");
			shadowPromisesTokenizer.tokenize(testPath);

			auto tokenIter = shadowPromisesTokenizer.tokens.begin();
			Assert::IsFalse(shadowPromisesTokenizer.tokens.empty(), L"There should be some tokens from the memory mapped file.");

			Assert::AreEqual("+'Line 1 of multi-line string\nLine 2 of multi-line string'"sv, tokenIter->tokenString);
			Assert::AreEqual((int)Token::TokenType::string, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);
		}
	};
}
