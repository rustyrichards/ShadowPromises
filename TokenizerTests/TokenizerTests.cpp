#include "pch.h"
#include <sstream>
#include <string>
#include <iostream>
#include "CppUnitTest.h"
#include "..\Parser\ShadowPromisesTokenizer.h"
#include "..\Parser\Parser.h"


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
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("|"sv, tokenIter->tokenString);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)Token::Token::assignment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)5, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("var1"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::Token::identifier, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)6, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# Assigning to var1"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::Token::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)12, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual(""sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::Token::endOfInput, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)31, tokenIter->startingCharacter);
		}

		TEST_METHOD(TokenizeGoodAndBadNumbers)
		{
			shadowPromisesTokenizer.cleanup();

			Logger::WriteMessage("In TokenizeGoodAndBadNumbers");
			auto source = std::istringstream(
				"1\n"
				"0\n"
				"7.012\n"
				"-5\n"
				"-.5\n"
				".5\n"
				"0xabcdef1234567890ABCDEF\n"
				"0X123\n"
				"-1.2e-2	#  -0.012 is valid\n"
				"01.01.23	# not a valid number\n"
				"--123		# not a valid number\n"
				"-			# not a valid number\n"
				".			# not a valid number\n"
				"0x			# no hex digits\n"
				"0xg		# g is not allowed\n"
				"0x1.1		# '.' is not allowed in a hex number\n"
			);

			shadowPromisesTokenizer.tokenize(source);

			auto tokenIter = shadowPromisesTokenizer.tokens.begin();
			Assert::AreEqual("1"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("0"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)2, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("7.012"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)3, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("-5"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)4, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("-.5"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)5, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual(".5"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)6, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("0xabcdef1234567890ABCDEF"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::hexNumber, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)7, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("0X123"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::hexNumber, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)8, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("-1.2e-2"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)9, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("#  -0.012 is valid"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)9, tokenIter->startingLine);

			tokenIter++;
			Assert::AreEqual("01.01.23"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::badNumber, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)10, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# not a valid number"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)10, tokenIter->startingLine);

			tokenIter++;
			Assert::AreEqual("--123"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::badNumber, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)11, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# not a valid number"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)11, tokenIter->startingLine);

			tokenIter++;
			Assert::AreEqual("-"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::badNumber, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)12, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# not a valid number"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)12, tokenIter->startingLine);

			tokenIter++;
			Assert::AreEqual("."sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::badNumber, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)13, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# not a valid number"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)13, tokenIter->startingLine);

			tokenIter++;
			Assert::AreEqual("0x"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::badNumber, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)14, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# no hex digits"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)14, tokenIter->startingLine);

			tokenIter++;
			Assert::AreEqual("0xg"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::badNumber, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)15, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# g is not allowed"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)15, tokenIter->startingLine);

			tokenIter++;
			Assert::AreEqual("0x1.1"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::badNumber, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)16, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# '.' is not allowed in a hex number"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)16, tokenIter->startingLine);
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
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("|"sv, tokenIter->tokenString);
			Assert::AreEqual((long)2, tokenIter->startingLine);
			Assert::AreEqual((long)Token::Token::assignment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("var1"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::Token::identifier, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)3, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual("# Assigning to var1"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::Token::comment, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)4, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual(""sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::Token::endOfInput, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)4, tokenIter->startingLine);
			Assert::AreEqual((long)20, tokenIter->startingCharacter);
		}

		TEST_METHOD(TokenizeAllTypes)
		{
			shadowPromisesTokenizer.cleanup();

			Logger::WriteMessage("In TokenizeAllTypes");
			auto source = std::istringstream(
				":define DEBUG\n"
				"\n"
				":option DEBUG {\n"
				"  2.0 | z\n"
				"}\n"
				"\n"
				":test(:equals(x y)) # :test and :if on separate lines\n"
				":if {\n"
				"  1.0 | z\n"
				"} 'simple string'\n"
				"\"Unterminated string.\n"
				":loop {\n"
				"  :test(true) :exit\n"
				"}\n"
				"\n"
				":undefine DEBUG\n"
			);

			shadowPromisesTokenizer.tokenize(source);

			auto tokenIter = shadowPromisesTokenizer.tokens.begin();
			Assert::AreEqual(":define"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("DEBUG"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":option"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("DEBUG"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("{"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::block_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("2.0"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("|"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::assignment, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("z"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("}"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::block_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":test"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("("sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::params_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":equals"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("("sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::params_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("x"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("y"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(")"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::params_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(")"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::params_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("# :test and :if on separate lines"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::comment, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":if"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("{"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::block_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("1.0"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::number, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("|"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::assignment, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("z"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("}"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::block_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("'simple string'"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::string, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("\"Unterminated string."sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::badString, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":loop"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("{"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::block_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":test"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("("sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::params_start, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("true"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(")"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::params_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":exit"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("}"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::block_end, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual(":undefine"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::Token::TokenType::keyword, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual("DEBUG"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::Token::TokenType::identifier, tokenIter->typeAndFlags.type);

			tokenIter++;
			Assert::AreEqual((long)Token::TokenType::endOfInput, tokenIter->typeAndFlags.type);
		}

		TEST_METHOD(TokenizeMultiLineString)
		{
			shadowPromisesTokenizer.cleanup();

			Logger::WriteMessage("In TokenizeMultiLineString");

			// A valid multi line string
			auto strBuffer = new string("+'Line 1 of multi-line string\n"
				"Line 2 of multi-line string' :test\n");

			shadowPromisesTokenizer.tokenize(string_view(strBuffer->c_str(), strBuffer->length()));

			auto tokenIter = shadowPromisesTokenizer.tokens.begin();
			Assert::AreEqual("+'Line 1 of multi-line string\nLine 2 of multi-line string'"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::string, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual(":test"sv, tokenIter->tokenString);
			Assert::AreEqual((long)Token::TokenType::keyword, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)2, tokenIter->startingLine);
			Assert::AreEqual((long)30, tokenIter->startingCharacter);

			tokenIter++;
			Assert::AreEqual((long)Token::TokenType::endOfInput, tokenIter->typeAndFlags.type);

			// Make sure the token.tokenString really just points into the source buffer.
			// Changing all the source charecers to ' ' should blank out the  tokens!
			auto runner = strBuffer->begin();
			auto end = strBuffer->end();
			while (runner != end)
			{
				if (0 != *runner) *runner = ' ';
				runner++;
			}

			// Now all the tokens should have as many ' ' as they had characters.
			tokenIter = shadowPromisesTokenizer.tokens.begin();
			Assert::AreEqual("                                                          "sv, tokenIter->tokenString);

			tokenIter++;
			Assert::AreEqual("     "sv, tokenIter->tokenString);

			shadowPromisesTokenizer.cleanup();

			// An unterminated multi-line string
			shadowPromisesTokenizer.tokenize(
				"+'Line 1 of multi-line string\n"
				"Line 2 of multi-line string :test\n"sv
			);

			auto tokenIter2 = shadowPromisesTokenizer.tokens.begin();
			Assert::AreEqual("+'Line 1 of multi-line string\nLine 2 of multi-line string :test\n"sv, tokenIter2->tokenString);
			Assert::AreEqual((long)Token::TokenType::badString, tokenIter2->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter2->startingLine);
			Assert::AreEqual((long)1, tokenIter2->startingCharacter);

			tokenIter2++;
			Assert::AreEqual((long)Token::TokenType::endOfInput, tokenIter2->typeAndFlags.type);
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
			Assert::AreEqual((long)Token::TokenType::string, tokenIter->typeAndFlags.type);
			Assert::AreEqual((long)1, tokenIter->startingLine);
			Assert::AreEqual((long)1, tokenIter->startingCharacter);
		}

		TEST_METHOD(ParserMemoryMappedFile)
		{
			shadowPromisesTokenizer.cleanup();


			Parser parser(shadowPromisesTokenizer);

			Logger::WriteMessage("In ParserMemoryMappedFile");

			// A file path (relative) to load as a memory mapped file.
			boost::filesystem::path testPath("TestCode.sp");
			auto startingNode = parser.parse(testPath);

			Assert::IsNotNull(startingNode);
		}
	};
}
