#include "parser.h"

#include <doctest/doctest.h>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

class ParserFixture
{
protected:
	template <class P, class... Args>
	bool test_parser(std::string_view input, const P& p, Args&... args)
	{
		using boost::spirit::qi::parse;

		auto f = input.begin();
		const auto l = input.end();
		return parse(f, l, p, args...) && f == l;
	}

	template <class P, class... Args>
	bool test_phrase_parser(std::string_view input, const P& p, Args&... args)
	{
		using boost::spirit::qi::phrase_parse;
		using boost::spirit::qi::ascii::space;

		auto f = input.begin();
		const auto l = input.end();
		return phrase_parse(f, l, p, space, args...) && f == l;
	}

	template <class P, class V>
	void test_value(std::string_view input, const P& p, const V& value)
	{
		V v;
		CHECK(test_parser(input, p, v));
		CHECK(v == value);
	}

	template <class P>
	void test_value(std::string_view input, const P& p, const char* value)
	{
		std::string v;
		CHECK(test_parser(input, p, v));
		CHECK(v == std::string(value));
	}

	Parser parser;
};

#define TEST_VALUE(a, b, c) \
	SUBCASE(a) { test_value(a, b, c); }

TEST_CASE_FIXTURE(ParserFixture, "keywords")
{
	std::string v;
	TEST_VALUE("and", parser.keyword, "and");
	TEST_VALUE("nil", parser.keyword, "nil");

	CHECK_FALSE(test_parser("toto", parser.keyword));
}

TEST_CASE_FIXTURE(ParserFixture, "names")
{
	TEST_VALUE("toto", parser.name, "toto");
	TEST_VALUE("_test", parser.name, "_test");
	TEST_VALUE("_123", parser.name, "_123");
	TEST_VALUE("_t1o2t3o4", parser.name, "_t1o2t3o4");

	CHECK_FALSE(test_parser("123toto", parser.name));
	CHECK_FALSE(test_parser("break", parser.name));
	CHECK_FALSE(test_parser("while", parser.name));
}

TEST_CASE_FIXTURE(ParserFixture, "short literal strings")
{
	TEST_VALUE("''", parser.shortLiteralString, "");
	TEST_VALUE("'test'", parser.shortLiteralString, "test");
	TEST_VALUE("\"test\"", parser.shortLiteralString, "test");
	TEST_VALUE("'test\" 123'", parser.shortLiteralString, "test\" 123");
	TEST_VALUE("\"test' 123\"", parser.shortLiteralString, "test' 123");
	TEST_VALUE("'test\\' 123'", parser.shortLiteralString, "test' 123");
	TEST_VALUE("\"test\\\" 123\"", parser.shortLiteralString, "test\" 123");
	TEST_VALUE("'line 1\r line 2'", parser.shortLiteralString, "line 1\r line 2");

	CHECK_FALSE(test_parser("no quotes here", parser.shortLiteralString));
	CHECK_FALSE(test_parser("'test", parser.shortLiteralString));
	CHECK_FALSE(test_parser("\"test'", parser.shortLiteralString));
}

TEST_CASE_FIXTURE(ParserFixture, "long literal strings")
{
	TEST_VALUE("[[]]", parser.longLiteralString, "");
	TEST_VALUE("[[test]]", parser.longLiteralString, "test");
	TEST_VALUE("[[test] 123]]", parser.longLiteralString, "test] 123");
	TEST_VALUE("[=[test]] 123]=]", parser.longLiteralString, "test]] 123");
	TEST_VALUE("[==[test]=] 123]==]", parser.longLiteralString, "test]=] 123");

	CHECK_FALSE(test_parser("[[test]", parser.longLiteralString));
	CHECK_FALSE(test_parser("[==[test]=]", parser.longLiteralString));
}

TEST_CASE_FIXTURE(ParserFixture, "literal strings")
{
	TEST_VALUE("'test'", parser.literalString, "test");
	TEST_VALUE("[[test]]", parser.literalString, "test");

	CHECK_FALSE(test_parser("test", parser.longLiteralString));
}

TEST_CASE_FIXTURE(ParserFixture, "numerals")
{
	TEST_VALUE("0", parser.numeral, 0.0);
	TEST_VALUE("-1", parser.numeral, -1.0);
	TEST_VALUE("0.1", parser.numeral, 0.1);
	TEST_VALUE("1e2", parser.numeral, 1e2);
	TEST_VALUE("1.2e3", parser.numeral, 1.2e3);
	TEST_VALUE("1.2e-3", parser.numeral, 1.2e-3);
	TEST_VALUE("0xa0", parser.numeral, 0xa0);
	TEST_VALUE("0Xa0", parser.numeral, 0xa0);
}

TEST_CASE_FIXTURE(ParserFixture, "comments")
{
	TEST_VALUE("--test", parser.comment, "test");
	TEST_VALUE("--test\n", parser.comment, "test");
	TEST_VALUE("--[test", parser.comment, "[test");
	TEST_VALUE("-- [[test]]", parser.comment, " [[test]]");
	TEST_VALUE("--[[test\n123]]", parser.comment, "test\n123");
	TEST_VALUE("--[=[test]]\n123]=]", parser.comment, "test]]\n123");

	CHECK_FALSE(test_parser("test", parser.comment));
}
