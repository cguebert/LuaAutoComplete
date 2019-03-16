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

	Parser parser;
};

TEST_CASE_FIXTURE(ParserFixture, "keywords")
{
	std::string v;
	CHECK(test_parser("and", parser.keyword, v));
	CHECK(v == "and");

	CHECK(test_parser("nil", parser.keyword, v));
	CHECK(v == "nil");

	CHECK_FALSE(test_parser("toto", parser.keyword));
}

TEST_CASE_FIXTURE(ParserFixture, "names")
{
	std::string v;
	CHECK(test_parser("toto", parser.name, v));
	CHECK(v == "toto");

	CHECK(test_parser("_test", parser.name, v));
	CHECK(v == "_test");

	CHECK(test_parser("_123", parser.name, v));
	CHECK(v == "_123");

	CHECK(test_parser("_t1o2t3o4", parser.name, v));
	CHECK(v == "_t1o2t3o4");

	CHECK_FALSE(test_parser("123toto", parser.name));

	CHECK_FALSE(test_parser("break", parser.name));
	CHECK_FALSE(test_parser("while", parser.name));
}

TEST_CASE_FIXTURE(ParserFixture, "short literal strings")
{
	std::string v;
	CHECK(test_parser("''", parser.shortLiteralString, v));
	CHECK(v == "");

	CHECK(test_parser("'test'", parser.shortLiteralString, v));
	CHECK(v == "test");

	CHECK(test_parser("\"test\"", parser.shortLiteralString, v));
	CHECK(v == "test");

	CHECK(test_parser("'test\" 123'", parser.shortLiteralString, v));
	CHECK(v == "test\" 123");

	CHECK(test_parser("\"test' 123\"", parser.shortLiteralString, v));
	CHECK(v == "test' 123");

	CHECK(test_parser("'test\\' 123'", parser.shortLiteralString, v));
	CHECK(v == "test' 123");

	CHECK(test_parser("\"test\\\" 123\"", parser.shortLiteralString, v));
	CHECK(v == "test\" 123");

	CHECK(test_parser("'line 1\r line 2'", parser.shortLiteralString, v));
	CHECK(v == "line 1\r line 2");

	CHECK_FALSE(test_parser("no quotes here", parser.shortLiteralString));
	CHECK_FALSE(test_parser("'test", parser.shortLiteralString));
	CHECK_FALSE(test_parser("\"test'", parser.shortLiteralString));
}
