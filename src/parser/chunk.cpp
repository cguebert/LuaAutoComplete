#include <parser/chunk_def.h>
#include <parser/config.h>
#include <parser/printer.h>

#include <doctest/doctest.h>
#include <iomanip>
#include <iostream>

namespace lac::parser
{
	BOOST_SPIRIT_INSTANTIATE(chunk_type, iterator_type, context_type);

	template <class P, class... Args>
	bool test_parser(std::string_view input, const P& p, Args&... args)
	{
		auto f = input.begin();
		const auto l = input.end();
		return boost::spirit::x3::parse(f, l, p, args...) && f == l;
	}

	template <class P, class... Args>
	bool test_phrase_parser(std::string_view input, const P& p, Args&... args)
	{
		auto f = input.begin();
		const auto l = input.end();
		return boost::spirit::x3::phrase_parse(f, l, p, boost::spirit::x3::ascii::space, args...) && f == l;
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

#define TEST_VALUE(a, b, c) \
	SUBCASE(a) { test_value(a, b, c); }

	using namespace lac::parser;

	TEST_CASE("keyword")
	{
		std::string v;
		TEST_VALUE("and", keyword, "and");
		TEST_VALUE("nil", keyword, "nil");

		CHECK_FALSE(test_parser("test", keyword));
	}

	TEST_CASE("name")
	{
		TEST_VALUE("test", name, "test");
		TEST_VALUE("_test", name, "_test");
		TEST_VALUE("_123", name, "_123");
		TEST_VALUE("_a1b2c3d4", name, "_a1b2c3d4");

		CHECK_FALSE(test_parser("123test", name));
		CHECK_FALSE(test_parser("test 123", name));
		CHECK_FALSE(test_phrase_parser("test 123", name));
		CHECK_FALSE(test_parser("break", name));
		CHECK_FALSE(test_parser("while", name));
	}

	TEST_CASE("namesList")
	{
		CHECK(test_phrase_parser("a", namesList));
		CHECK(test_phrase_parser("a, b", namesList));
		CHECK(test_phrase_parser("a, b, c", namesList));

		CHECK_FALSE(test_phrase_parser("a, b,", namesList));

		std::vector<std::string> list;
		REQUIRE(test_phrase_parser("a, b, c", namesList, list));
		REQUIRE(list.size() == 3);
		CHECK(list[0] == "a");
		CHECK(list[1] == "b");
		CHECK(list[2] == "c");
	}

	TEST_CASE("short literal string")
	{
		CHECK(test_parser("''", literalString));

		TEST_VALUE("''", literalString, "");
		TEST_VALUE("'test'", literalString, "test");
		TEST_VALUE("\"test\"", literalString, "test");
		TEST_VALUE("'test\" 123'", literalString, "test\" 123");
		TEST_VALUE("\"test' 123\"", literalString, "test' 123");
		TEST_VALUE("'test\\' 123'", literalString, "test' 123");
		TEST_VALUE("\"test\\\" 123\"", literalString, "test\" 123");
		TEST_VALUE("'line 1\r line 2'", literalString, "line 1\r line 2");

		CHECK_FALSE(test_parser("no quotes here", literalString));
		CHECK_FALSE(test_parser("'test", literalString));
		CHECK_FALSE(test_parser("\"test'", literalString));

		CHECK(test_phrase_parser("'test 1'", literalString));
		CHECK(test_phrase_parser("'test 1\t 2'", literalString));

		std::string v;
		CHECK(test_phrase_parser("'test 1 \t2 3 4'", literalString, v));
		CHECK(v == std::string("test 1 \t2 3 4"));
	}

	TEST_CASE("long literal string")
	{
		TEST_VALUE("[[]]", literalString, "");
		TEST_VALUE("[[test]]", literalString, "test");
		TEST_VALUE("[[test] 123]]", literalString, "test] 123");
		TEST_VALUE("[=[test]] 123]=]", literalString, "test]] 123");
		TEST_VALUE("[==[test]=] 123]==]", literalString, "test]=] 123");

		CHECK_FALSE(test_parser("test", literalString));
		CHECK_FALSE(test_parser("[[test]", literalString));
		CHECK_FALSE(test_parser("[==[test]=]", literalString));
	}

	TEST_CASE("literal string")
	{
		TEST_VALUE("'test'", literalString, "test");
		TEST_VALUE("[[test]]", literalString, "test");
	}

	TEST_CASE("numeral")
	{
		TEST_VALUE("0", numeral, 0.0);
		TEST_VALUE("-1", numeral, -1.0);
		TEST_VALUE("0.1", numeral, 0.1);
		TEST_VALUE("1e2", numeral, 1e2);
		TEST_VALUE("1.2e3", numeral, 1.2e3);
		TEST_VALUE("1.2e-3", numeral, 1.2e-3);
		TEST_VALUE("0xa0", numeral, double(0xa0));
		TEST_VALUE("0Xa0", numeral, double(0xa0));
	}

	TEST_CASE("numeral as string")
	{
		TEST_VALUE("0", numeralAsString, "0");
		TEST_VALUE("-1", numeralAsString, "-1");
		TEST_VALUE("0.1", numeralAsString, "0.1");
		TEST_VALUE("1e2", numeralAsString, "1e2");
		TEST_VALUE("1.2e3", numeralAsString, "1.2e3");
		TEST_VALUE("1.2e-3", numeralAsString, "1.2e-3");
		TEST_VALUE("0xa0", numeralAsString, "0xa0");
		TEST_VALUE("0Xa0", numeralAsString, "0Xa0");
	}

	TEST_CASE("comment")
	{
		TEST_VALUE("--test", comment, "test");
		TEST_VALUE("--test\n", comment, "test");
		TEST_VALUE("--[test", comment, "[test");
		TEST_VALUE("-- [[test]]", comment, " [[test]]");
		TEST_VALUE("--[[test\n123]]", comment, "test\n123");
		TEST_VALUE("--[=[test]]\n123]=]", comment, "test]]\n123");

		SUBCASE("short comment value") {
			std::string v;
			CHECK(test_phrase_parser("-- test 1 2", comment, v));
			CHECK(v == std::string("test 1 2"));
		}

		SUBCASE("long comment value") {
			std::string v;
			CHECK(test_phrase_parser("--[[test 1 2]]", comment, v));
			CHECK(v == std::string("test 1 2"));
		}

		CHECK_FALSE(test_parser("test", comment));
	}
	
	TEST_CASE("field by expression")
	{
		CHECK(test_phrase_parser("[2] = 0", fieldByExpression));
		CHECK(test_phrase_parser("['test'] = 42", fieldByExpression));
		CHECK(test_phrase_parser("['hello' .. 'World'] = '!'", fieldByExpression));

		ast::FieldByExpression fe;
		REQUIRE(test_phrase_parser("['hello' .. 'World'] = 42", fieldByExpression, fe));
		REQUIRE(fe.key.operand.get().type() == typeid(std::string));
		CHECK(boost::get<std::string>(fe.key.operand) == "hello");

		REQUIRE(fe.key.binaryOperation.is_initialized());
		CHECK(fe.key.binaryOperation->get().operation == ast::Operation::concat);
		REQUIRE(fe.key.binaryOperation->get().expression.operand.get().type() == typeid(std::string));
		CHECK(boost::get<std::string>(fe.key.binaryOperation->get().expression.operand) == "World");

		REQUIRE(fe.value.operand.get().type() == typeid(double));
		CHECK(boost::get<double>(fe.value.operand) == 42.0);
	}

	TEST_CASE("field by assignement")
	{
		CHECK(test_phrase_parser("test = 0", fieldByAssignement));
		CHECK(test_phrase_parser("x = 'test'", fieldByAssignement));

		ast::FieldByAssignement fa;
		REQUIRE(test_phrase_parser("x = 'test'", fieldByAssignement, fa));
		CHECK(fa.name == "x");
		REQUIRE(fa.value.operand.get().type() == typeid(std::string));
		CHECK(boost::get<std::string>(fa.value.operand) == "test");
	}

	TEST_CASE("field")
	{
		CHECK(test_phrase_parser("[2] = 2", field));
		CHECK(test_phrase_parser("[f(x)] = a", field));
		CHECK(test_phrase_parser("x = 1", field));
		CHECK(test_phrase_parser("x", field));
		CHECK(test_phrase_parser("42", field));
		CHECK(test_phrase_parser("f(x)", field));

		SUBCASE("by expression") {
			ast::Field f;
			REQUIRE(test_phrase_parser("['x'] = 42", field, f));
			REQUIRE(f.get().type() == typeid(ast::FieldByExpression));
			auto fe = boost::get<ast::FieldByExpression>(f.get());
			CHECK(boost::get<std::string>(fe.key.operand) == "x");
			REQUIRE(fe.value.operand.get().type() == typeid(double));
			CHECK(boost::get<double>(fe.value.operand) == 42.0);
		}

		SUBCASE("by assignment") {
			ast::Field f;
			REQUIRE(test_phrase_parser("x = 42", field, f));
			REQUIRE(f.get().type() == typeid(ast::FieldByAssignement));
			auto fa = boost::get<ast::FieldByAssignement>(f.get());
			CHECK(boost::get<std::string>(fa.name) == "x");
			REQUIRE(fa.value.operand.get().type() == typeid(double));
			CHECK(boost::get<double>(fa.value.operand) == 42.0);
		}

		SUBCASE("expression only") {
			ast::Field f;
			REQUIRE(test_phrase_parser("'x'", field, f));
			REQUIRE(f.get().type() == typeid(ast::Expression));
			auto ex = boost::get<ast::Expression>(f.get());
			CHECK(ex.operand.get().type() == typeid(std::string));
			CHECK(boost::get<std::string>(ex.operand) == "x");
		}
	}
	
	TEST_CASE("fieldsList")
	{
		CHECK(test_phrase_parser("1, 2, 3", fieldsList));
		CHECK(test_phrase_parser("1; 2; 3", fieldsList));
		CHECK(test_phrase_parser("1, 2; 3", fieldsList));
		CHECK(test_phrase_parser("1, 2; 3,", fieldsList));
		CHECK(test_phrase_parser("1, 2; 3;", fieldsList));
		CHECK(test_phrase_parser("x", fieldsList));
		CHECK(test_phrase_parser("x,", fieldsList));
		CHECK(test_phrase_parser("[2] = 2, 2, x, x = 1", fieldsList));
		CHECK(test_phrase_parser("[2] = 2; 2, x; x = 1", fieldsList));
		CHECK(test_phrase_parser("[2] = 2, 2; x; x = 1;", fieldsList));
		CHECK(test_phrase_parser("[2] = 2, 2; x; x = 1,", fieldsList));
	}
	
	TEST_CASE("tableConstructor")
	{
		CHECK(test_phrase_parser("{}", tableConstructor));
		CHECK(test_phrase_parser("{1, 2, 3}", tableConstructor));
		CHECK(test_phrase_parser("{'hello', 'World', 42}", tableConstructor));
		CHECK(test_phrase_parser("{x}", tableConstructor));
		CHECK(test_phrase_parser("{ x }", tableConstructor));
		CHECK(test_phrase_parser("{[2] = 2, 2, x, x = 1}", tableConstructor));
		CHECK(test_phrase_parser("{ [2] = 2, 2, x, x = 1 }", tableConstructor));
		CHECK(test_phrase_parser("{ [2] = 2; 2, x; x = 1 }", tableConstructor));
		CHECK(test_phrase_parser("{[2] = 2, 2; x; x = 1;}", tableConstructor));
		CHECK(test_phrase_parser("{[2] = 2, 2; x; x = 1,}", tableConstructor));
	}
	
	TEST_CASE("parametersList")
	{
		CHECK(test_phrase_parser("...", parametersList));
		CHECK(test_phrase_parser("a", parametersList));
		CHECK(test_phrase_parser("a, ...", parametersList));
		CHECK(test_phrase_parser("a, b, c", parametersList));
		CHECK(test_phrase_parser("a, b, c, ...", parametersList));

		CHECK_FALSE(test_phrase_parser("a,", parametersList));
		CHECK_FALSE(test_phrase_parser("a, b...", parametersList));

		SUBCASE("multiple parameters") {
			ast::ParametersList pl;
			REQUIRE(test_phrase_parser("a, b, c", parametersList, pl));
			CHECK(pl.varargs == false);
			REQUIRE(pl.parameters.size() == 3);
			CHECK(pl.parameters[0] == "a");
			CHECK(pl.parameters[1] == "b");
			CHECK(pl.parameters[2] == "c");
		}

		SUBCASE("one parameter with varargs") {
			ast::ParametersList pl;
			REQUIRE(test_phrase_parser("a, ...", parametersList, pl));
			CHECK(pl.varargs == true);
			REQUIRE(pl.parameters.size() == 1);
			CHECK(pl.parameters[0] == "a");
		}

		SUBCASE("only varargs") {
			ast::ParametersList pl;
			REQUIRE(test_phrase_parser("...", parametersList, pl));
			CHECK(pl.varargs == true);
			REQUIRE(pl.parameters.size() == 0);
		}
	}
	/*
	TEST_CASE("functionBody")
	{
		CHECK(test_phrase_parser("() end", functionBody));
		CHECK(test_phrase_parser("(a) x=1 end", functionBody));
	}

	TEST_CASE("functionDefinition")
	{
		CHECK(test_phrase_parser("function () end", functionDefinition));
		CHECK(test_phrase_parser("function (a) x=a + 1; return x; end", functionDefinition));
	}
	*/
	TEST_CASE("arguments")
	{
		CHECK(test_phrase_parser("()", arguments));
		CHECK(test_phrase_parser("(1, 2)", arguments));
		CHECK(test_phrase_parser("(x)", arguments));
		CHECK(test_phrase_parser("(x, 42, 'test')", arguments));
		CHECK(test_phrase_parser("{x=1}", arguments));
		CHECK(test_phrase_parser("'test'", arguments));

		SUBCASE("empty arguments") {
			ast::Arguments ar;
			REQUIRE(test_phrase_parser("()", arguments, ar));
			CHECK(ar.get().type() == typeid(ast::EmptyArguments));
		}

		SUBCASE("arguments expressions") {
			ast::Arguments ar;
			REQUIRE(test_phrase_parser("(1, 2)", arguments, ar));
			REQUIRE(ar.get().type() == typeid(ast::ExpressionsList));
			auto el = boost::get<ast::ExpressionsList>(ar);
			CHECK(el.size() == 2);
		}

		SUBCASE("table") {
			ast::Arguments ar;
			REQUIRE(test_phrase_parser("{1, 2}", arguments, ar));
			REQUIRE(ar.get().type() == typeid(ast::TableConstructor));
			auto tc = boost::get<ast::TableConstructor>(ar);
			REQUIRE(tc.fields.is_initialized());
			CHECK(tc.fields->size() == 2);
		}
	}
	
	TEST_CASE("functionCall")
	{
		CHECK(test_phrase_parser("func()", functionCall));
		CHECK(test_phrase_parser("var:member()", functionCall));
		CHECK(test_phrase_parser("func(x)", functionCall));
		CHECK(test_phrase_parser("func(x, 42, 'test')", functionCall));
		CHECK(test_phrase_parser("func {x, x=1}", functionCall));
		CHECK(test_phrase_parser("func '42'", functionCall));

		CHECK_FALSE(test_phrase_parser("func", functionCall));

		SUBCASE("no member") {
			ast::FunctionCall fc;
			REQUIRE(test_phrase_parser("func()", functionCall, fc));
			REQUIRE(fc.variable.start.get().type() == typeid(std::string));
			CHECK(boost::get<std::string>(fc.variable.start) == "func");
			CHECK(fc.functionCall.member.is_initialized() == false);
		}

		SUBCASE("with member") {
			ast::FunctionCall fc;
			REQUIRE(test_phrase_parser("func:member()", functionCall, fc));
			REQUIRE(fc.variable.start.get().type() == typeid(std::string));
			CHECK(boost::get<std::string>(fc.variable.start) == "func");
			REQUIRE(fc.functionCall.member.is_initialized());
			CHECK(fc.functionCall.member.get() == "member");
		}
	}
	
	TEST_CASE("functionCallEnd")
	{
		CHECK(test_phrase_parser("()", functionCallEnd));
		CHECK(test_phrase_parser(":member()", functionCallEnd));
		CHECK(test_phrase_parser(":member(42)", functionCallEnd));
		CHECK(test_phrase_parser("(42)", functionCallEnd));

		SUBCASE("no member") {
			ast::FunctionCallEnd fce;
			REQUIRE(test_phrase_parser("(42)", functionCallEnd, fce));
			CHECK(fce.member.is_initialized() == false);
		}

		SUBCASE("with member") {
			ast::FunctionCallEnd fce;
			REQUIRE(test_phrase_parser(":member(42)", functionCallEnd, fce));
			REQUIRE(fce.member.is_initialized());
			CHECK(fce.member.get() == "member");
		}
	}
	
	TEST_CASE("postPrefixExpression")
	{
		SUBCASE("table index name") {
			ast::PostPrefix pp;
			REQUIRE(test_phrase_parser(".b", postPrefix, pp));
			CHECK(pp.get().type() == typeid(ast::TableIndexName));
		}

		SUBCASE("table index expression") {
			ast::PostPrefix pp;
			REQUIRE(test_phrase_parser("[42]", postPrefix, pp));
			CHECK(pp.get().type() == typeid(ast::TableIndexExpression));
		}

		SUBCASE("function call end") {
			ast::PostPrefix pp;
			REQUIRE(test_phrase_parser("(42)", postPrefix, pp));
			CHECK(pp.get().type() == typeid(ast::FunctionCallEnd));
		}
	}

	TEST_CASE("prefixExpression")
	{
		CHECK(test_phrase_parser("x", prefixExpression));
		CHECK(test_phrase_parser("(true)", prefixExpression));
		CHECK(test_phrase_parser("(42)", prefixExpression));
		CHECK(test_phrase_parser("('test')", prefixExpression));
		CHECK(test_phrase_parser("(...)", prefixExpression));
		CHECK(test_phrase_parser("a", prefixExpression));
		CHECK(test_phrase_parser("a.b", prefixExpression));
		CHECK(test_phrase_parser("a.b('c')", prefixExpression));
		CHECK(test_phrase_parser("a.b('c'):d()", prefixExpression));
		CHECK(test_phrase_parser("a.b('c'):d(42)", prefixExpression));

		SUBCASE("bracketed expression") {
			ast::PrefixExpression pe;
			REQUIRE(test_phrase_parser("(42)", prefixExpression, pe));
			CHECK(pe.start.get().type() == typeid(ast::BracketedExpression));
		}

		SUBCASE("name") {
			ast::PrefixExpression pe;
			REQUIRE(test_phrase_parser("a", prefixExpression, pe));
			CHECK(pe.start.get().type() == typeid(std::string));
		}

		SUBCASE("rest") {
			ast::PrefixExpression pe;
			REQUIRE(test_phrase_parser("a.b.c", prefixExpression, pe));
			CHECK(pe.rest.size() == 2);
		}
	}
	
	TEST_CASE("expression")
	{
		CHECK(test_phrase_parser("nil", expression));
		CHECK(test_phrase_parser("false", expression));
		CHECK(test_phrase_parser("true", expression));
		CHECK(test_phrase_parser("...", expression));
		CHECK(test_phrase_parser("42", expression));
		CHECK(test_phrase_parser("'test'", expression));
		CHECK(test_phrase_parser("\"test\"", expression));
		CHECK(test_phrase_parser("[[test]]", expression));
		CHECK(test_phrase_parser("{}", expression));
		CHECK(test_phrase_parser("{1, 2}", expression));
		CHECK(test_phrase_parser("{['test'] = 42, 3.14}", expression));
	//	CHECK(test_phrase_parser("function(a, b, c) return a + b / c; end", expression));
		CHECK(test_phrase_parser("x", expression));
		CHECK(test_phrase_parser("a.b.c", expression));
		CHECK(test_phrase_parser("a[b][c]", expression));
		CHECK(test_phrase_parser("a.b[c]", expression));
		CHECK(test_phrase_parser("a[b].c", expression));
		CHECK(test_phrase_parser("(exp)[b]", expression));
		CHECK(test_phrase_parser("(exp).b", expression));
		CHECK(test_phrase_parser("func(a, b).c", expression));
		CHECK(test_phrase_parser("func(a, b)[c]", expression));
		CHECK(test_phrase_parser("func(a, b)[c]:test('hello').d:e(f)", expression));
		CHECK(test_phrase_parser("a[-2]", expression));
		CHECK(test_phrase_parser("a.x[1]", expression));
		CHECK(test_phrase_parser("a.b.c", expression));
		CHECK(test_phrase_parser("a + 2", expression));
		CHECK(test_phrase_parser("a or 2", expression));
		CHECK(test_phrase_parser("2 + a / 3", expression));
		CHECK(test_phrase_parser("(a + 2) / 3", expression));
		CHECK(test_phrase_parser("(a + (-y)) / (3 - x)", expression));
		CHECK(test_phrase_parser("-2", expression));
		CHECK(test_phrase_parser("{f(x)}", expression));
		CHECK(test_phrase_parser("func()", expression));
		CHECK(test_phrase_parser("func(x)", expression));
		CHECK(test_phrase_parser("func(x, 42, 'test')", expression));
		CHECK(test_phrase_parser("func{x, x=1}", expression));
		CHECK(test_phrase_parser("func'42'", expression));

		SUBCASE("expression constant") {
			ast::Expression ex;
			REQUIRE(test_phrase_parser("nil", expression, ex));
			REQUIRE(ex.operand.get().type() == typeid(ast::ExpressionConstant));
			CHECK(boost::get<ast::ExpressionConstant>(ex.operand) == ast::ExpressionConstant::nil);
		}

		SUBCASE("numeral") {
			ast::Expression ex;
			REQUIRE(test_phrase_parser("42.3", expression, ex));
			REQUIRE(ex.operand.get().type() == typeid(double));
			CHECK(boost::get<double>(ex.operand) == 42.3);
		}

		SUBCASE("string") {
			ast::Expression ex;
			REQUIRE(test_phrase_parser("'test'", expression, ex));
			REQUIRE(ex.operand.get().type() == typeid(std::string));
			CHECK(boost::get<std::string>(ex.operand) == "test");
		}

		SUBCASE("unary operation") {
			ast::Expression ex;
			REQUIRE(test_phrase_parser("not true", expression, ex));
			REQUIRE(ex.operand.get().type() == typeid(ast::f_UnaryOperation));
			auto uo = boost::get<ast::f_UnaryOperation>(ex.operand.get()).get();
			CHECK(uo.operation == ast::Operation::bnot);
			REQUIRE(uo.expression.operand.get().type() == typeid(ast::ExpressionConstant));
			CHECK(boost::get<ast::ExpressionConstant>(uo.expression.operand) == ast::ExpressionConstant::True);
		}

		SUBCASE("binary operation") {
			ast::Expression ex;
			REQUIRE(test_phrase_parser("1 + 2", expression, ex));
			REQUIRE(ex.operand.get().type() == typeid(double));
			CHECK(boost::get<double>(ex.operand) == 1);

			REQUIRE(ex.binaryOperation.is_initialized());
			const auto bo = ex.binaryOperation->get();
			CHECK(bo.operation == ast::Operation::add);
			REQUIRE(bo.expression.operand.get().type() == typeid(double));
			CHECK(boost::get<double>(bo.expression.operand) == 2);
		}

		SUBCASE("table constructor") {
			ast::Expression ex;
			REQUIRE(test_phrase_parser("{1, 2}", expression, ex));
			REQUIRE(ex.operand.get().type() == typeid(ast::TableConstructor));
		}
	}

	TEST_CASE("expressionsList")
	{
		CHECK(test_phrase_parser("nil", expressionsList));
		CHECK(test_phrase_parser("nil, true, false", expressionsList));
		CHECK(test_phrase_parser("true, 42, 'test', ...", expressionsList));

		CHECK_FALSE(test_phrase_parser("nil,", expressionsList));
	}

	TEST_CASE("bracketed expression")
	{
		CHECK(test_phrase_parser("(42)", bracketedExpression));
		CHECK(test_phrase_parser("(1 + 2)", bracketedExpression));
		CHECK(test_phrase_parser("('hello' .. 'world')", bracketedExpression));
		CHECK(test_phrase_parser("(x + 1)", bracketedExpression));

		CHECK_FALSE(test_phrase_parser("42", bracketedExpression));
		CHECK_FALSE(test_phrase_parser("()", bracketedExpression));

		SUBCASE("expression inside parentheses") {
			ast::BracketedExpression be;
			REQUIRE(test_phrase_parser("(42)", bracketedExpression, be));
			REQUIRE(be.expression.operand.get().type() == typeid(double));
			CHECK(boost::get<double>(be.expression.operand) == 42);
		}
	}

	TEST_CASE("table index expression")
	{
		CHECK(test_phrase_parser("['hello']", tableIndexExpression));
		CHECK(test_phrase_parser("[42]", tableIndexExpression));
		CHECK(test_phrase_parser("[42 + 3.14]", tableIndexExpression));
		CHECK(test_phrase_parser("[x + 1]", tableIndexExpression));

		CHECK_FALSE(test_phrase_parser("[]", tableIndexExpression));

		SUBCASE("expression") {
			ast::TableIndexExpression tie;
			REQUIRE(test_phrase_parser("[42]", tableIndexExpression, tie));
			REQUIRE(tie.expression.operand.get().type() == typeid(double));
			CHECK(boost::get<double>(tie.expression.operand) == 42);
		}
	}

	TEST_CASE("table index name")
	{
		CHECK(test_phrase_parser(".x", tableIndexName));
		CHECK(test_phrase_parser(".test", tableIndexName));

		CHECK_FALSE(test_phrase_parser(".", tableIndexName));
		CHECK_FALSE(test_phrase_parser(".42", tableIndexName));

		SUBCASE("name") {
			ast::TableIndexName tin;
			REQUIRE(test_phrase_parser(".test", tableIndexName, tin));
			CHECK(tin.name == "test");
		}
	}

	TEST_CASE("variableFunctionCall")
	{
		CHECK(test_phrase_parser("().a", variableFunctionCall));
		CHECK(test_phrase_parser(":a().b", variableFunctionCall));

		CHECK_FALSE(test_phrase_parser("a().b", variableFunctionCall));
		CHECK_FALSE(test_phrase_parser("(a)", variableFunctionCall));
	}

	TEST_CASE("variablePostfix")
	{
		SUBCASE("table index name") {
			ast::VariablePostfix vp;
			REQUIRE(test_phrase_parser(".b", variablePostfix, vp));
			CHECK(vp.get().type() == typeid(ast::TableIndexName));
		}

		SUBCASE("table index expression") {
			ast::VariablePostfix vp;
			REQUIRE(test_phrase_parser("[42]", variablePostfix, vp));
			CHECK(vp.get().type() == typeid(ast::TableIndexExpression));
		}

		SUBCASE("function call end") {
			ast::VariablePostfix vp;
			REQUIRE(test_phrase_parser("(42).a", variablePostfix, vp));
			CHECK(vp.get().type() == typeid(ast::f_VariableFunctionCall));
		}
	}

	TEST_CASE("variable")
	{
		CHECK(test_phrase_parser("a", variable));
		CHECK(test_phrase_parser("a[2]", variable));
		CHECK(test_phrase_parser("a[2].b", variable));
		CHECK(test_phrase_parser("a.b", variable));
		CHECK(test_phrase_parser("a.b().c", variable));
		CHECK(test_phrase_parser("(a).b", variable));
		CHECK(test_phrase_parser("a(b)[c]", variable));
		CHECK(test_phrase_parser("a(b):d().e", variable));
		CHECK(test_phrase_parser("a(b)[c]:d().e", variable));
		CHECK(test_phrase_parser("a(b)[c]:d()[e]", variable));

		CHECK_FALSE(test_phrase_parser("a.b()", variable));
		CHECK_FALSE(test_phrase_parser("a(b)", variable));
		CHECK_FALSE(test_phrase_parser("a(b):d()", variable));
		CHECK_FALSE(test_phrase_parser("a(b)[c]:d()", variable));
	}
	
	TEST_CASE("variablesList")
	{
		CHECK(test_phrase_parser("a, b, c", variablesList));
		CHECK(test_phrase_parser("a, b:c().d, e[2]", variablesList));
		CHECK(test_phrase_parser("a.b, c(d).e", variablesList));
	}
	
	TEST_CASE("functionName")
	{
		CHECK(test_phrase_parser("test", functionName));
		CHECK(test_phrase_parser("test.a", functionName));
		CHECK(test_phrase_parser("test.a.b", functionName));
		CHECK(test_phrase_parser("test:c", functionName));
		CHECK(test_phrase_parser("test.a.b:c", functionName));

		CHECK_FALSE(test_phrase_parser(".a", functionName));
		CHECK_FALSE(test_phrase_parser(":c", functionName));
		CHECK_FALSE(test_phrase_parser(".a:c", functionName));

		SUBCASE("one name") {
			ast::FunctionName fn;
			REQUIRE(test_phrase_parser("func", functionName, fn));
			CHECK(fn.start == "func");
			CHECK(fn.rest.size() == 0);
			CHECK(fn.member.is_initialized() == false);
		}

		SUBCASE("multiple names") {
			ast::FunctionName fn;
			REQUIRE(test_phrase_parser("a.b.c", functionName, fn));
			CHECK(fn.start == "a");
			REQUIRE(fn.rest.size() == 2);
			CHECK(fn.rest[0] == "b");
			CHECK(fn.rest[1] == "c");
			CHECK(fn.member.is_initialized() == false);
		}

		SUBCASE("multiple names and member") {
			ast::FunctionName fn;
			REQUIRE(test_phrase_parser("a.b.c:d", functionName, fn));
			CHECK(fn.start == "a");
			REQUIRE(fn.rest.size() == 2);
			CHECK(fn.rest[0] == "b");
			CHECK(fn.rest[1] == "c");
			REQUIRE(fn.member.is_initialized() == true);
			CHECK(fn.member->name == "d");
		}
	}
	/*
	TEST_CASE("label")
	{
		CHECK(test_phrase_parser("::test::", label));
		CHECK(test_phrase_parser(":: test ::", label));

		CHECK_FALSE(test_phrase_parser("::test:", label));
		CHECK_FALSE(test_phrase_parser(":: test 123 ::", label));
		CHECK_FALSE(test_phrase_parser("::::", label));
	}

	TEST_CASE("returnStatement")
	{
		CHECK(test_phrase_parser("return", returnStatement));
		CHECK(test_phrase_parser("return;", returnStatement));
		CHECK(test_phrase_parser("return 42", returnStatement));
		CHECK(test_phrase_parser("return 42;", returnStatement));
		CHECK(test_phrase_parser("return x", returnStatement));
		CHECK(test_phrase_parser("return x;", returnStatement));
		CHECK(test_phrase_parser("return 1, x;", returnStatement));
		CHECK(test_phrase_parser("return func(42), 'hello'", returnStatement));
	}

	TEST_CASE("statement")
	{
		CHECK(test_phrase_parser(";", statement));
		CHECK(test_phrase_parser("x = 2", statement));
		CHECK(test_phrase_parser("a, b = 2.34, 42", statement));
		CHECK(test_phrase_parser("a(b)", statement));
		CHECK(test_phrase_parser("a:b(c, 42)", statement));
		CHECK(test_phrase_parser("::test::", statement));
		CHECK(test_phrase_parser("break", statement));
		CHECK(test_phrase_parser("goto test", statement));
		CHECK(test_phrase_parser("do x = x+1 end", statement));
		CHECK(test_phrase_parser("while x < 10 do x = x + 1 end", statement));
		CHECK(test_phrase_parser("repeat x = x + 1 until x > 10", statement));
		CHECK(test_phrase_parser("if x < 10 then x = x + 1 else x = x - 1 end", statement));
		CHECK(test_phrase_parser("if x < 10 then x = x + 1 elseif x > 20 then x = x - 1 end", statement));
		CHECK(test_phrase_parser("for x = 0, 10 do print(x) end", statement));
		CHECK(test_phrase_parser("for x in 1, 2, 3 do print(x) end", statement));
		CHECK(test_phrase_parser("function test() return 42 end", statement));
		CHECK(test_phrase_parser("local function test() return 42 end", statement));
		CHECK(test_phrase_parser("local x = 42", statement));
		CHECK(test_phrase_parser("local a, b = 2.34, 42", statement));

		CHECK_FALSE(test_phrase_parser("test", statement));
	}*/

	TEST_CASE("Printer")
	{
		ast::Expression ex;
		REQUIRE(test_phrase_parser("'hello' + 42 / 3.15 * - 2", expression, ex));

		std::cout << "---------------\n";
		print(ex);
		std::cout << "\n---------------\n";
	}
} // namespace lac::parser
