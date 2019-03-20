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
		auto f = input.begin();
		const auto l = input.end();
		return boost::spirit::qi::parse(f, l, p, args...) && f == l;
	}

	template <class P, class... Args>
	bool test_phrase_parser(std::string_view input, const P& p, Args&... args)
	{
		auto f = input.begin();
		const auto l = input.end();
		return boost::spirit::qi::phrase_parse(f, l, p, parser.skipper, args...) && f == l;
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

TEST_CASE_FIXTURE(ParserFixture, "keyword")
{
	std::string v;
	TEST_VALUE("and", parser.keyword, "and");
	TEST_VALUE("nil", parser.keyword, "nil");

	CHECK_FALSE(test_parser("test", parser.keyword));
}

TEST_CASE_FIXTURE(ParserFixture, "name")
{
	TEST_VALUE("test", parser.name, "test");
	TEST_VALUE("_test", parser.name, "_test");
	TEST_VALUE("_123", parser.name, "_123");
	TEST_VALUE("_a1b2c3d4", parser.name, "_a1b2c3d4");

	CHECK_FALSE(test_parser("123test", parser.name));
	CHECK_FALSE(test_parser("test 123", parser.name));
	CHECK_FALSE(test_parser("break", parser.name));
	CHECK_FALSE(test_parser("while", parser.name));
}

TEST_CASE_FIXTURE(ParserFixture, "short literal string")
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

TEST_CASE_FIXTURE(ParserFixture, "long literal string")
{
	TEST_VALUE("[[]]", parser.longLiteralString, "");
	TEST_VALUE("[[test]]", parser.longLiteralString, "test");
	TEST_VALUE("[[test] 123]]", parser.longLiteralString, "test] 123");
	TEST_VALUE("[=[test]] 123]=]", parser.longLiteralString, "test]] 123");
	TEST_VALUE("[==[test]=] 123]==]", parser.longLiteralString, "test]=] 123");

	CHECK_FALSE(test_parser("test", parser.longLiteralString));
	CHECK_FALSE(test_parser("[[test]", parser.longLiteralString));
	CHECK_FALSE(test_parser("[==[test]=]", parser.longLiteralString));
}

TEST_CASE_FIXTURE(ParserFixture, "literal string")
{
	TEST_VALUE("'test'", parser.literalString, "test");
	TEST_VALUE("[[test]]", parser.literalString, "test");
}

TEST_CASE_FIXTURE(ParserFixture, "numeral")
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

TEST_CASE_FIXTURE(ParserFixture, "numeral as string")
{
	TEST_VALUE("0", parser.numeralAsString, "0");
	TEST_VALUE("-1", parser.numeralAsString, "-1");
	TEST_VALUE("0.1", parser.numeralAsString, "0.1");
	TEST_VALUE("1e2", parser.numeralAsString, "1e2");
	TEST_VALUE("1.2e3", parser.numeralAsString, "1.2e3");
	TEST_VALUE("1.2e-3", parser.numeralAsString, "1.2e-3");
	TEST_VALUE("0xa0", parser.numeralAsString, "0xa0");
	TEST_VALUE("0Xa0", parser.numeralAsString, "0Xa0");
}

TEST_CASE_FIXTURE(ParserFixture, "comment")
{
	TEST_VALUE("--test", parser.comment, "test");
	TEST_VALUE("--test\n", parser.comment, "test");
	TEST_VALUE("--[test", parser.comment, "[test");
	TEST_VALUE("-- [[test]]", parser.comment, " [[test]]");
	TEST_VALUE("--[[test\n123]]", parser.comment, "test\n123");
	TEST_VALUE("--[=[test]]\n123]=]", parser.comment, "test]]\n123");

	CHECK_FALSE(test_parser("test", parser.comment));
}

TEST_CASE_FIXTURE(ParserFixture, "field")
{
	CHECK(test_phrase_parser("[2] = 2", parser.field));
	CHECK(test_phrase_parser("[f(x)] = a", parser.field));
	CHECK(test_phrase_parser("x = 1", parser.field));
	CHECK(test_phrase_parser("x", parser.field));
	CHECK(test_phrase_parser("42", parser.field));
	CHECK(test_phrase_parser("f(x)", parser.field));
}

TEST_CASE_FIXTURE(ParserFixture, "fieldsList")
{
	CHECK(test_phrase_parser("[2] = 2, 2, x, x = 1", parser.fieldsList));
	CHECK(test_phrase_parser("[2] = 2; 2, x; x = 1", parser.fieldsList));
	CHECK(test_phrase_parser("x,", parser.fieldsList));
	CHECK(test_phrase_parser("[2] = 2, 2; x; x = 1;", parser.fieldsList));
	CHECK(test_phrase_parser("[2] = 2, 2; x; x = 1,", parser.fieldsList));
}

TEST_CASE_FIXTURE(ParserFixture, "tableConstructor")
{
	CHECK(test_phrase_parser("{}", parser.tableConstructor));
	CHECK(test_phrase_parser("{x}", parser.tableConstructor));
	CHECK(test_phrase_parser("{ x }", parser.tableConstructor));
	CHECK(test_phrase_parser("{[2] = 2, 2, x, x = 1}", parser.tableConstructor));
	CHECK(test_phrase_parser("{ [2] = 2, 2, x, x = 1 }", parser.tableConstructor));
	CHECK(test_phrase_parser("{ [2] = 2; 2, x; x = 1 }", parser.tableConstructor));
	CHECK(test_phrase_parser("{[2] = 2, 2; x; x = 1;}", parser.tableConstructor));
	CHECK(test_phrase_parser("{[2] = 2, 2; x; x = 1,}", parser.tableConstructor));
}

TEST_CASE_FIXTURE(ParserFixture, "parametersList")
{
	CHECK(test_phrase_parser("...", parser.parametersList));
	CHECK(test_phrase_parser("a", parser.parametersList));
	CHECK(test_phrase_parser("a, ...", parser.parametersList));
	CHECK(test_phrase_parser("a, b, c", parser.parametersList));
	CHECK(test_phrase_parser("a, b, c, ...", parser.parametersList));

	CHECK_FALSE(test_phrase_parser("a,", parser.parametersList));
	CHECK_FALSE(test_phrase_parser("a, b...", parser.parametersList));
}

TEST_CASE_FIXTURE(ParserFixture, "functionBody")
{
	CHECK(test_phrase_parser("() end", parser.functionBody));
	CHECK(test_phrase_parser("(a) x=1 end", parser.functionBody));
}

TEST_CASE_FIXTURE(ParserFixture, "functionDefinition")
{
	CHECK(test_phrase_parser("function () end", parser.functionDefinition));
	CHECK(test_phrase_parser("function (a) x=a + 1; return x; end", parser.functionDefinition));
}

TEST_CASE_FIXTURE(ParserFixture, "arguments")
{
	CHECK(test_phrase_parser("()", parser.arguments));
	CHECK(test_phrase_parser("(x)", parser.arguments));
	CHECK(test_phrase_parser("(x, 42, 'test')", parser.arguments));
	CHECK(test_phrase_parser("{x=1}", parser.arguments));
	CHECK(test_phrase_parser("'test'", parser.arguments));
}

TEST_CASE_FIXTURE(ParserFixture, "functionCall")
{
	CHECK(test_phrase_parser("func()", parser.functionCall));
	CHECK(test_phrase_parser("func(x)", parser.functionCall));
	CHECK(test_phrase_parser("func(x, 42, 'test')", parser.functionCall));
	CHECK(test_phrase_parser("func {x, x=1}", parser.functionCall));
	CHECK(test_phrase_parser("func '42'", parser.functionCall));

	CHECK_FALSE(test_phrase_parser("func", parser.functionCall));
}

TEST_CASE_FIXTURE(ParserFixture, "prefixExpression")
{
	CHECK(test_phrase_parser("x", parser.prefixExpression));
	CHECK(test_phrase_parser("(true)", parser.prefixExpression));
	CHECK(test_phrase_parser("(42)", parser.prefixExpression));
	CHECK(test_phrase_parser("('test')", parser.prefixExpression));
	CHECK(test_phrase_parser("(...)", parser.prefixExpression));
}

TEST_CASE_FIXTURE(ParserFixture, "expression")
{
	CHECK(test_phrase_parser("nil", parser.expression));
	CHECK(test_phrase_parser("false", parser.expression));
	CHECK(test_phrase_parser("true", parser.expression));
	CHECK(test_phrase_parser("42", parser.expression));
	CHECK(test_phrase_parser("'test'", parser.expression));
	CHECK(test_phrase_parser("...", parser.expression));
	CHECK(test_phrase_parser("function(a, b, c) return a + b / c; end", parser.expression));
	CHECK(test_phrase_parser("x", parser.expression));
	CHECK(test_phrase_parser("a.b.c", parser.expression));
	CHECK(test_phrase_parser("a[b][c]", parser.expression));
	CHECK(test_phrase_parser("a.b[c]", parser.expression));
	CHECK(test_phrase_parser("a[b].c", parser.expression));
	CHECK(test_phrase_parser("(exp)[b]", parser.expression));
	CHECK(test_phrase_parser("(exp).b", parser.expression));
	CHECK(test_phrase_parser("func(a, b).c", parser.expression));
	CHECK(test_phrase_parser("func(a, b)[c]", parser.expression));
	CHECK(test_phrase_parser("func(a, b)[c]:test('hello').d:e(f)", parser.expression));
	CHECK(test_phrase_parser("{}", parser.expression));
	CHECK(test_phrase_parser("{x}", parser.expression));
	CHECK(test_phrase_parser("{x; x=2}", parser.expression));
	CHECK(test_phrase_parser("a[-2]", parser.expression));
	CHECK(test_phrase_parser("a.x[1]", parser.expression));
	CHECK(test_phrase_parser("a.b.c", parser.expression));
	CHECK(test_phrase_parser("a + 2", parser.expression));
	CHECK(test_phrase_parser("a or 2", parser.expression));
	CHECK(test_phrase_parser("2 + a / 3", parser.expression));
	CHECK(test_phrase_parser("(a + 2) / 3", parser.expression));
	CHECK(test_phrase_parser("(a + (-y)) / (3 - x)", parser.expression));
	CHECK(test_phrase_parser("-2", parser.expression));
	CHECK(test_phrase_parser("{f(x)}", parser.expression));
	CHECK(test_phrase_parser("func()", parser.expression));
	CHECK(test_phrase_parser("func(x)", parser.expression));
	CHECK(test_phrase_parser("func(x, 42, 'test')", parser.expression));
	CHECK(test_phrase_parser("func{x, x=1}", parser.expression));
	CHECK(test_phrase_parser("func'42'", parser.expression));
}

TEST_CASE_FIXTURE(ParserFixture, "expressionsList")
{
	CHECK(test_phrase_parser("nil", parser.expressionsList));
	CHECK(test_phrase_parser("nil, true, false", parser.expressionsList));
	CHECK(test_phrase_parser("true, 42, 'test', ...", parser.expressionsList));

	CHECK_FALSE(test_phrase_parser("nil,", parser.expressionsList));
}

TEST_CASE_FIXTURE(ParserFixture, "namesList")
{
	CHECK(test_phrase_parser("a", parser.namesList));
	CHECK(test_phrase_parser("a, b", parser.namesList));
	CHECK(test_phrase_parser("a, b, c", parser.namesList));

	CHECK_FALSE(test_phrase_parser("a, b,", parser.namesList));
}

TEST_CASE_FIXTURE(ParserFixture, "variable")
{
	CHECK(test_phrase_parser("a", parser.variable));
	CHECK(test_phrase_parser("a[2]", parser.variable));
	CHECK(test_phrase_parser("a[2].b", parser.variable));
	CHECK(test_phrase_parser("a.b", parser.variable));
	CHECK(test_phrase_parser("a.b().c", parser.variable));
	CHECK(test_phrase_parser("(a).b", parser.variable));
	CHECK(test_phrase_parser("a(b)[c]", parser.variable));
	CHECK(test_phrase_parser("a(b):d().e", parser.variable));
	CHECK(test_phrase_parser("a(b)[c]:d().e", parser.variable));
	CHECK(test_phrase_parser("a(b)[c]:d()[e]", parser.variable));

	CHECK_FALSE(test_phrase_parser("a.b()", parser.variable));
	CHECK_FALSE(test_phrase_parser("a(b)", parser.variable));
	CHECK_FALSE(test_phrase_parser("a(b):d()", parser.variable));
	CHECK_FALSE(test_phrase_parser("a(b)[c]:d()", parser.variable));
}

TEST_CASE_FIXTURE(ParserFixture, "variablesList")
{
	CHECK(test_phrase_parser("a, b, c", parser.variablesList));
	CHECK(test_phrase_parser("a, b:c().d, e[2]", parser.variablesList));
	CHECK(test_phrase_parser("a.b, c(d).e", parser.variablesList));
}

TEST_CASE_FIXTURE(ParserFixture, "functionName")
{
	CHECK(test_phrase_parser("test", parser.functionName));
	CHECK(test_phrase_parser("test.a", parser.functionName));
	CHECK(test_phrase_parser("test.a.b", parser.functionName));
	CHECK(test_phrase_parser("test:c", parser.functionName));
	CHECK(test_phrase_parser("test.a.b:c", parser.functionName));

	CHECK_FALSE(test_phrase_parser(".a", parser.functionName));
	CHECK_FALSE(test_phrase_parser(":c", parser.functionName));
	CHECK_FALSE(test_phrase_parser(".a:c", parser.functionName));
}

TEST_CASE_FIXTURE(ParserFixture, "label")
{
	CHECK(test_phrase_parser("::test::", parser.label));
	CHECK(test_phrase_parser(":: test ::", parser.label));

	CHECK_FALSE(test_phrase_parser("::test:", parser.label));
	CHECK_FALSE(test_phrase_parser(":: test 123 ::", parser.label));
	CHECK_FALSE(test_phrase_parser("::::", parser.label));
}

TEST_CASE_FIXTURE(ParserFixture, "returnStatement")
{
	CHECK(test_phrase_parser("return", parser.returnStatement));
	CHECK(test_phrase_parser("return;", parser.returnStatement));
	CHECK(test_phrase_parser("return 42", parser.returnStatement));
	CHECK(test_phrase_parser("return 42;", parser.returnStatement));
	CHECK(test_phrase_parser("return x", parser.returnStatement));
	CHECK(test_phrase_parser("return x;", parser.returnStatement));
	CHECK(test_phrase_parser("return 1, x;", parser.returnStatement));
	CHECK(test_phrase_parser("return func(42), 'hello'", parser.returnStatement));
}

TEST_CASE_FIXTURE(ParserFixture, "statement")
{
	CHECK(test_phrase_parser(";", parser.statement));
	CHECK(test_phrase_parser("x = 2", parser.statement));
	CHECK(test_phrase_parser("a, b = 2.34, 42", parser.statement));
	CHECK(test_phrase_parser("a(b)", parser.statement));
	CHECK(test_phrase_parser("a:b(c, 42)", parser.statement));
	CHECK(test_phrase_parser("::test::", parser.statement));
	CHECK(test_phrase_parser("break", parser.statement));
	CHECK(test_phrase_parser("goto test", parser.statement));
	CHECK(test_phrase_parser("do x = x+1 end", parser.statement));
	CHECK(test_phrase_parser("while x < 10 do x = x + 1 end", parser.statement));
	CHECK(test_phrase_parser("repeat x = x + 1 until x > 10", parser.statement));
	CHECK(test_phrase_parser("if x < 10 then x = x + 1 else x = x - 1 end", parser.statement));
	CHECK(test_phrase_parser("if x < 10 then x = x + 1 elseif x > 20 then x = x - 1 end", parser.statement));
	CHECK(test_phrase_parser("for x = 0, 10 do print(x) end", parser.statement));
	CHECK(test_phrase_parser("for x in 1, 2, 3 do print(x) end", parser.statement));
	CHECK(test_phrase_parser("function test() return 42 end", parser.statement));
	CHECK(test_phrase_parser("local function test() return 42 end", parser.statement));
	CHECK(test_phrase_parser("local x = 42", parser.statement));
	CHECK(test_phrase_parser("local a, b = 2.34, 42", parser.statement));

	CHECK_FALSE(test_phrase_parser("test", parser.statement));
}

TEST_CASE_FIXTURE(ParserFixture, "chunk")
{
	Parser::ChunkGrammar g(parser);
	CHECK(test_phrase_parser("x = 2", g));
	CHECK(test_phrase_parser("x = 2 --comment", g));
	CHECK(test_phrase_parser("x = {}", g));

	CHECK_FALSE(test_phrase_parser("x = 2 3", g));

//	CHECK_FALSE(test_phrase_parser("test", parser.statement, ascii::space));
}
