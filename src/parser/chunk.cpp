#include <parser/chunk_def.h>
#include <parser/config.h>
#include <parser/positions.h>

#include <helper/test_utils.h>
#include <iostream>

namespace lac::ast
{
	bool operator==(const LiteralString& lhs, const LiteralString& rhs)
	{
		return lhs.value == rhs.value;
	}
} // namespace lac::ast

namespace lac::parser
{
	BOOST_SPIRIT_INSTANTIATE(chunk_type, iterator_type, no_skip_pos_context_type);
	BOOST_SPIRIT_INSTANTIATE(chunk_type, iterator_type, skipper_context_type);
	BOOST_SPIRIT_INSTANTIATE(chunk_type, iterator_type, chunk_pos_context_type);
	//	BOOST_SPIRIT_INSTANTIATE(chunk_type, iterator_type, skipper_pos_context_type); // Not used
	BOOST_SPIRIT_INSTANTIATE(variable_or_function_type, iterator_type, skipper_context_type);

	using helper::test_parser;
	using helper::test_parser_simple;
	using helper::test_phrase_parser;
	using helper::test_phrase_parser_simple;
	using helper::test_value;

	TEST_CASE("keyword")
	{
		std::string v;
		TEST_VALUE("and", keyword, "and");
		TEST_VALUE("nil", keyword, "nil");

		CHECK_FALSE(test_parser_simple("test", keyword));
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

	ast::LiteralString operator""_lit(const char* str, size_t len)
	{
		ast::LiteralString tmp;
		tmp.value = std::string{str, len};
		return tmp;
	}

	TEST_CASE("short literal string")
	{
		CHECK(test_parser("''", literalString));

		TEST_VALUE("''", literalString, ""_lit);
		TEST_VALUE("'test'", literalString, "test"_lit);
		TEST_VALUE("\"test\"", literalString, "test"_lit);
		TEST_VALUE("'test\" 123'", literalString, "test\" 123"_lit);
		TEST_VALUE("\"test' 123\"", literalString, "test' 123"_lit);
		TEST_VALUE("'test\\' 123'", literalString, "test' 123"_lit);
		TEST_VALUE("\"test\\\" 123\"", literalString, "test\" 123"_lit);
		TEST_VALUE("'line 1\r line 2'", literalString, "line 1\r line 2"_lit);

		CHECK_FALSE(test_parser("no quotes here", literalString));
		CHECK_FALSE(test_parser("'test", literalString));
		CHECK_FALSE(test_parser("\"test'", literalString));

		CHECK(test_phrase_parser("'test 1'", literalString));
		CHECK(test_phrase_parser("'test 1\t 2'", literalString));

		ast::LiteralString v;
		CHECK(test_phrase_parser("'test 1 \t2 3 4'", literalString, v));
		CHECK(v.value == std::string("test 1 \t2 3 4"));
	}

	TEST_CASE("long literal string")
	{
		TEST_VALUE("[[]]", literalString, ""_lit);
		TEST_VALUE("[[test]]", literalString, "test"_lit);
		TEST_VALUE("[[test] 123]]", literalString, "test] 123"_lit);
		TEST_VALUE("[=[test]] 123]=]", literalString, "test]] 123"_lit);
		TEST_VALUE("[==[test]=] 123]==]", literalString, "test]=] 123"_lit);

		CHECK_FALSE(test_parser("test", literalString));
		CHECK_FALSE(test_parser("[[test]", literalString));
		CHECK_FALSE(test_parser("[==[test]=]", literalString));
	}

	TEST_CASE("literal string")
	{
		TEST_VALUE("'test'", literalString, "test"_lit);
		TEST_VALUE("[[test]]", literalString, "test"_lit);
	}

	TEST_CASE("numeral int")
	{
		TEST_VALUE("0", numeralInt, 0);
		TEST_VALUE("42", numeralInt, 42);
		TEST_VALUE("-1", numeralInt, -1);
		TEST_VALUE("0xa0", numeralInt, int(0xa0));
		TEST_VALUE("0Xa0", numeralInt, int(0xa0));

		CHECK_FALSE(test_parser("0.0", numeralInt));
	}

	TEST_CASE("numeral float")
	{
		TEST_VALUE("0", numeralFloat, 0.0);
		TEST_VALUE("0.1", numeralFloat, 0.1);
		TEST_VALUE("3.14", numeralFloat, 3.14);
		TEST_VALUE("-3.14", numeralFloat, -3.14);
		TEST_VALUE("1e2", numeralFloat, 1e2);
		TEST_VALUE("1.2e3", numeralFloat, 1.2e3);
		TEST_VALUE("1.2e-3", numeralFloat, 1.2e-3);

		CHECK(test_phrase_parser("3.14", numeralFloat));
		CHECK(test_phrase_parser("42.3", numeralFloat));

		CHECK_FALSE(test_phrase_parser("42,3", numeralFloat));
	}

	TEST_CASE("comment")
	{
		TEST_VALUE("--test", comment, "test");
		TEST_VALUE("--test\n", comment, "test");
		TEST_VALUE("--[test", comment, "[test");
		TEST_VALUE("-- [[test]]", comment, " [[test]]");
		TEST_VALUE("--[[test\n123]]", comment, "test\n123");
		TEST_VALUE("--[=[test]]\n123]=]", comment, "test]]\n123");

		SUBCASE("short comment value")
		{
			std::string v;
			CHECK(test_phrase_parser("-- test 1 2", comment, v));
			CHECK(v == std::string("test 1 2"));
		}

		SUBCASE("long comment value")
		{
			std::string v;
			CHECK(test_phrase_parser("--[[test 1 2]]", comment, v));
			CHECK(v == std::string("test 1 2"));
		}

		CHECK_FALSE(test_parser("test", comment));
	}

	TEST_CASE("skipper")
	{
		CHECK(test_parser("-- test", skipper));
		CHECK(test_parser("-- test\n", skipper));
		CHECK(test_parser("--[[ ]]", skipper));
		CHECK(test_parser("--[[ comment ]]", skipper));
		CHECK(test_parser("--[[ this is a comment ]]", skipper));
		CHECK(test_parser("--[[ this a long\tcomment ]]", skipper));
		CHECK(test_parser("--[[\n]]", skipper));
		CHECK(test_parser("--[[ this a very\nlong comment ]]", skipper));
		CHECK(test_parser("--[[ this a \tvery\nlong\tcomment ]]", skipper));
		CHECK(test_parser(" ", skipper));
		CHECK(test_parser("\n", skipper));
		CHECK(test_parser("\t", skipper));
	}

	TEST_CASE("field by expression")
	{
		CHECK(test_phrase_parser("[2] = 0", fieldByExpression));
		CHECK(test_phrase_parser("['test'] = 42", fieldByExpression));
		CHECK(test_phrase_parser("['hello' .. 'World'] = '!'", fieldByExpression));

		ast::FieldByExpression fe;
		REQUIRE(test_phrase_parser("['hello' .. 'World'] = 42", fieldByExpression, fe));
		REQUIRE(fe.key.operand.isLiteral());
		CHECK(fe.key.operand.asLiteral().value == "hello");

		REQUIRE(fe.key.binaryOperation.is_initialized());
		CHECK(fe.key.binaryOperation->get().operation == ast::Operation::concat);
		REQUIRE(fe.key.binaryOperation->get().expression.operand.isLiteral());
		CHECK(fe.key.binaryOperation->get().expression.operand.asLiteral().value == "World");

		REQUIRE(fe.value.operand.isNumeral());
		const auto num = fe.value.operand.asNumeral();
		REQUIRE(num.isInt());
		CHECK(num.asInt() == 42);
	}

	TEST_CASE("field by assignment")
	{
		CHECK(test_phrase_parser("test = 0", fieldByAssignment));
		CHECK(test_phrase_parser("x = 'test'", fieldByAssignment));

		ast::FieldByAssignment fa;
		REQUIRE(test_phrase_parser("x = 'test'", fieldByAssignment, fa));
		CHECK(fa.name == "x");
		REQUIRE(fa.value.operand.isLiteral());
		CHECK(fa.value.operand.asLiteral().value == "test");
	}

	TEST_CASE("field")
	{
		CHECK(test_phrase_parser("[2] = 2", field));
		CHECK(test_phrase_parser("[f(x)] = a", field));
		CHECK(test_phrase_parser("x = 1", field));
		CHECK(test_phrase_parser("x", field));
		CHECK(test_phrase_parser("42", field));
		CHECK(test_phrase_parser("f(x)", field));

		SUBCASE("by expression")
		{
			ast::Field f;
			REQUIRE(test_phrase_parser("['x'] = 42", field, f));
			REQUIRE(f.get().type() == typeid(ast::FieldByExpression));
			auto fe = boost::get<ast::FieldByExpression>(f.get());
			REQUIRE(fe.key.operand.isLiteral());
			CHECK(fe.key.operand.asLiteral().value == "x");
			REQUIRE(fe.value.operand.isNumeral());
			REQUIRE(fe.value.operand.asNumeral().isInt());
			CHECK(fe.value.operand.asNumeral().asInt() == 42);
		}

		SUBCASE("by assignment")
		{
			ast::Field f;
			REQUIRE(test_phrase_parser("x = 42", field, f));
			REQUIRE(f.get().type() == typeid(ast::FieldByAssignment));
			auto fa = boost::get<ast::FieldByAssignment>(f.get());
			CHECK(fa.name == "x");
			REQUIRE(fa.value.operand.isNumeral());
			REQUIRE(fa.value.operand.asNumeral().isInt());
			CHECK(fa.value.operand.asNumeral().asInt() == 42);
		}

		SUBCASE("expression only")
		{
			ast::Field f;
			REQUIRE(test_phrase_parser("'x'", field, f));
			REQUIRE(f.get().type() == typeid(ast::Expression));
			auto ex = boost::get<ast::Expression>(f.get());
			CHECK(ex.operand.isLiteral());
			CHECK(ex.operand.asLiteral().value == "x");
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

		SUBCASE("multiple parameters")
		{
			ast::ParametersList pl;
			REQUIRE(test_phrase_parser("a, b, c", parametersList, pl));
			CHECK(pl.varargs == false);
			REQUIRE(pl.parameters.size() == 3);
			CHECK(pl.parameters[0] == "a");
			CHECK(pl.parameters[1] == "b");
			CHECK(pl.parameters[2] == "c");
		}

		SUBCASE("one parameter with varargs")
		{
			ast::ParametersList pl;
			REQUIRE(test_phrase_parser("a, ...", parametersList, pl));
			CHECK(pl.varargs == true);
			REQUIRE(pl.parameters.size() == 1);
			CHECK(pl.parameters[0] == "a");
		}

		SUBCASE("only varargs")
		{
			ast::ParametersList pl;
			REQUIRE(test_phrase_parser("...", parametersList, pl));
			CHECK(pl.varargs == true);
			REQUIRE(pl.parameters.size() == 0);
		}
	}

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

	TEST_CASE("arguments")
	{
		CHECK(test_phrase_parser("()", arguments));
		CHECK(test_phrase_parser("(1, 2)", arguments));
		CHECK(test_phrase_parser("(x)", arguments));
		CHECK(test_phrase_parser("(x, 42, 'test')", arguments));
		CHECK(test_phrase_parser("{x=1}", arguments));
		CHECK(test_phrase_parser("'test'", arguments));

		SUBCASE("empty arguments")
		{
			ast::Arguments ar;
			REQUIRE(test_phrase_parser("()", arguments, ar));
			CHECK(ar.get().type() == typeid(ast::EmptyArguments));
		}

		SUBCASE("arguments expressions")
		{
			ast::Arguments ar;
			REQUIRE(test_phrase_parser("(1, 2)", arguments, ar));
			REQUIRE(ar.get().type() == typeid(ast::ExpressionsList));
			auto el = boost::get<ast::ExpressionsList>(ar);
			CHECK(el.size() == 2);
		}

		SUBCASE("table")
		{
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

		SUBCASE("no member")
		{
			ast::FunctionCall fc;
			REQUIRE(test_phrase_parser("func()", functionCall, fc));
			REQUIRE(fc.variable.start.get().type() == typeid(std::string));
			CHECK(boost::get<std::string>(fc.variable.start) == "func");
			CHECK(fc.functionCall.member.is_initialized() == false);
		}

		SUBCASE("with member")
		{
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

		SUBCASE("no member")
		{
			ast::FunctionCallEnd fce;
			REQUIRE(test_phrase_parser("(42)", functionCallEnd, fce));
			CHECK(fce.member.is_initialized() == false);
		}

		SUBCASE("with member")
		{
			ast::FunctionCallEnd fce;
			REQUIRE(test_phrase_parser(":member(42)", functionCallEnd, fce));
			REQUIRE(fce.member.is_initialized());
			CHECK(fce.member.get() == "member");
		}
	}

	TEST_CASE("postPrefixExpression")
	{
		SUBCASE("table index name")
		{
			ast::PostPrefix pp;
			REQUIRE(test_phrase_parser(".b", postPrefix, pp));
			CHECK(pp.get().type() == typeid(ast::TableIndexName));
		}

		SUBCASE("table index expression")
		{
			ast::PostPrefix pp;
			REQUIRE(test_phrase_parser("[42]", postPrefix, pp));
			CHECK(pp.get().type() == typeid(ast::TableIndexExpression));
		}

		SUBCASE("function call end")
		{
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

		SUBCASE("bracketed expression")
		{
			ast::PrefixExpression pe;
			REQUIRE(test_phrase_parser("(42)", prefixExpression, pe));
			CHECK(pe.start.get().type() == typeid(ast::BracketedExpression));
		}

		SUBCASE("name")
		{
			ast::PrefixExpression pe;
			REQUIRE(test_phrase_parser("a", prefixExpression, pe));
			CHECK(pe.start.get().type() == typeid(std::string));
		}

		SUBCASE("rest")
		{
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
		CHECK(test_phrase_parser("3.14", expression));
		CHECK(test_phrase_parser("'test'", expression));
		CHECK(test_phrase_parser("\"test\"", expression));
		CHECK(test_phrase_parser("[[test]]", expression));
		CHECK(test_phrase_parser("{}", expression));
		CHECK(test_phrase_parser("{1, 2}", expression));
		CHECK(test_phrase_parser("{['test'] = 42, 3.14}", expression));
		CHECK(test_phrase_parser("function(a, b, c) return a + b / c; end", expression));
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

		SUBCASE("expression constant")
		{
			ast::Expression ex;
			REQUIRE(test_phrase_parser("nil", expression, ex));
			REQUIRE(ex.operand.get().type() == typeid(ast::ExpressionConstant));
			CHECK(boost::get<ast::ExpressionConstant>(ex.operand) == ast::ExpressionConstant::nil);
		}

		SUBCASE("numeral int")
		{
			ast::Expression ex;
			REQUIRE(test_phrase_parser("42", expression, ex));
			REQUIRE(ex.operand.isNumeral());
			REQUIRE(ex.operand.asNumeral().isInt());
			CHECK(ex.operand.asNumeral().asInt() == 42);
		}

		SUBCASE("numeral float")
		{
			ast::Expression ex;
			REQUIRE(test_phrase_parser("42.3", expression, ex));
			REQUIRE(ex.operand.isNumeral());
			REQUIRE(ex.operand.asNumeral().isFloat());
			CHECK(ex.operand.asNumeral().asFloat() == 42.3);
		}

		SUBCASE("string")
		{
			ast::Expression ex;
			REQUIRE(test_phrase_parser("'test'", expression, ex));
			REQUIRE(ex.operand.isLiteral());
			CHECK(ex.operand.asLiteral().value == "test");
		}

		SUBCASE("unary operation")
		{
			ast::Expression ex;
			REQUIRE(test_phrase_parser("not true", expression, ex));
			REQUIRE(ex.operand.get().type() == typeid(ast::f_UnaryOperation));
			auto uo = boost::get<ast::f_UnaryOperation>(ex.operand.get()).get();
			CHECK(uo.operation == ast::Operation::lnot);
			REQUIRE(uo.expression.operand.get().type() == typeid(ast::ExpressionConstant));
			CHECK(boost::get<ast::ExpressionConstant>(uo.expression.operand) == ast::ExpressionConstant::True);
		}

		SUBCASE("binary operation")
		{
			ast::Expression ex;
			REQUIRE(test_phrase_parser("1 + 2", expression, ex));
			REQUIRE(ex.operand.isNumeral());
			REQUIRE(ex.operand.asNumeral().isInt());
			CHECK(ex.operand.asNumeral().asInt() == 1);

			REQUIRE(ex.binaryOperation.is_initialized());
			const auto bo = ex.binaryOperation->get();
			CHECK(bo.operation == ast::Operation::add);
			REQUIRE(bo.expression.operand.isNumeral());
			REQUIRE(bo.expression.operand.asNumeral().isInt());
			CHECK(bo.expression.operand.asNumeral().asInt() == 2);
		}

		SUBCASE("function definition")
		{
			ast::Expression ex;
			REQUIRE(test_phrase_parser("function(a, b, c) return a + b / c; end", expression, ex));
			REQUIRE(ex.operand.get().type() == typeid(ast::f_FunctionBody));
		}

		SUBCASE("table constructor")
		{
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

		SUBCASE("expression inside parentheses")
		{
			ast::BracketedExpression be;
			REQUIRE(test_phrase_parser("(42)", bracketedExpression, be));
			REQUIRE(be.expression.operand.isNumeral());
			REQUIRE(be.expression.operand.asNumeral().isInt());
			CHECK(be.expression.operand.asNumeral().asInt() == 42);
		}
	}

	TEST_CASE("table index expression")
	{
		CHECK(test_phrase_parser("['hello']", tableIndexExpression));
		CHECK(test_phrase_parser("[42]", tableIndexExpression));
		CHECK(test_phrase_parser("[42 + 3.14]", tableIndexExpression));
		CHECK(test_phrase_parser("[x + 1]", tableIndexExpression));

		CHECK_FALSE(test_phrase_parser("[]", tableIndexExpression));

		SUBCASE("expression")
		{
			ast::TableIndexExpression tie;
			REQUIRE(test_phrase_parser("[42]", tableIndexExpression, tie));
			REQUIRE(tie.expression.operand.isNumeral());
			REQUIRE(tie.expression.operand.asNumeral().isInt());
			CHECK(tie.expression.operand.asNumeral().asInt() == 42);
		}
	}

	TEST_CASE("table index name")
	{
		CHECK(test_phrase_parser(".x", tableIndexName));
		CHECK(test_phrase_parser(".test", tableIndexName));

		CHECK_FALSE(test_phrase_parser(".", tableIndexName));
		CHECK_FALSE(test_phrase_parser(".42", tableIndexName));

		SUBCASE("name")
		{
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
		SUBCASE("table index name")
		{
			ast::VariablePostfix vp;
			REQUIRE(test_phrase_parser(".b", variablePostfix, vp));
			CHECK(vp.get().type() == typeid(ast::TableIndexName));
		}

		SUBCASE("table index expression")
		{
			ast::VariablePostfix vp;
			REQUIRE(test_phrase_parser("[42]", variablePostfix, vp));
			CHECK(vp.get().type() == typeid(ast::TableIndexExpression));
		}

		SUBCASE("function call end")
		{
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

		SUBCASE("one name")
		{
			ast::FunctionName fn;
			REQUIRE(test_phrase_parser("func", functionName, fn));
			CHECK(fn.start == "func");
			CHECK(fn.rest.size() == 0);
			CHECK(fn.member.is_initialized() == false);
		}

		SUBCASE("multiple names")
		{
			ast::FunctionName fn;
			REQUIRE(test_phrase_parser("a.b.c", functionName, fn));
			CHECK(fn.start == "a");
			REQUIRE(fn.rest.size() == 2);
			CHECK(fn.rest[0] == "b");
			CHECK(fn.rest[1] == "c");
			CHECK(fn.member.is_initialized() == false);
		}

		SUBCASE("multiple names and member")
		{
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

	// This is used for the completion, not part of the Lua parser
	TEST_CASE("variable or function")
	{
		CHECK(test_phrase_parser("test", variableOrFunction));
		CHECK(test_phrase_parser("a", variableOrFunction));
		CHECK(test_phrase_parser("a[2]", variableOrFunction));
		CHECK(test_phrase_parser("a[2].b", variableOrFunction));
		CHECK(test_phrase_parser("a.b", variableOrFunction));
		CHECK(test_phrase_parser("a.b().c", variableOrFunction));
		CHECK(test_phrase_parser("(a).b", variableOrFunction));
		CHECK(test_phrase_parser("a(b)[c]", variableOrFunction));
		CHECK(test_phrase_parser("a(b):d().e", variableOrFunction));
		CHECK(test_phrase_parser("a(b)[c]:d().e", variableOrFunction));
		CHECK(test_phrase_parser("a(b)[c]:d()[e]", variableOrFunction));

		CHECK(test_phrase_parser("a:b", variableOrFunction));
		CHECK(test_phrase_parser("a(b)[c]:d", variableOrFunction));

		CHECK_FALSE(test_phrase_parser("a(b)", variableOrFunction));
		CHECK_FALSE(test_phrase_parser("a(b):c", variableOrFunction));
	}

	TEST_CASE("empty statement")
	{
		CHECK(test_phrase_parser_simple(";", emptyStatement));
	}

	TEST_CASE("assignment statement")
	{
		CHECK(test_phrase_parser("x = 1", assignmentStatement));
		CHECK(test_phrase_parser("x = 42 + 3.14", assignmentStatement));
		CHECK(test_phrase_parser("x, y = 'hello', 'World'", assignmentStatement));

		SUBCASE("one variable")
		{
			ast::AssignmentStatement as;
			REQUIRE(test_phrase_parser("x = 42", assignmentStatement, as));
			REQUIRE(as.variables.size() == 1);
			REQUIRE(as.expressions.size() == 1);
			const auto& var = as.variables[0];
			REQUIRE(var.start.get().type() == typeid(std::string));
			CHECK(boost::get<std::string>(var.start) == "x");
			const auto& exp = as.expressions[0];
			REQUIRE(exp.operand.isNumeral());
			REQUIRE(exp.operand.asNumeral().isInt());
			REQUIRE(exp.operand.asNumeral().asInt() == 42);
		}

		SUBCASE("two variables")
		{
			ast::AssignmentStatement as;
			REQUIRE(test_phrase_parser("x, y = 42, 'hello'", assignmentStatement, as));
			REQUIRE(as.variables.size() == 2);
			REQUIRE(as.expressions.size() == 2);

			const auto& x = as.variables[0];
			REQUIRE(x.start.get().type() == typeid(std::string));
			CHECK(boost::get<std::string>(x.start) == "x");

			const auto& y = as.variables[1];
			REQUIRE(y.start.get().type() == typeid(std::string));
			CHECK(boost::get<std::string>(y.start) == "y");

			const auto& exp1 = as.expressions[0];
			REQUIRE(exp1.operand.isNumeral());
			REQUIRE(exp1.operand.asNumeral().isInt());
			REQUIRE(exp1.operand.asNumeral().asInt() == 42);

			const auto& exp2 = as.expressions[1];
			REQUIRE(exp2.operand.isLiteral());
			CHECK(exp2.operand.asLiteral().value == "hello");
		}
	}

	TEST_CASE("local assignment statement")
	{
		CHECK(test_phrase_parser("local x", localAssignmentStatement));
		CHECK(test_phrase_parser("local x, y", localAssignmentStatement));
		CHECK(test_phrase_parser("local x = 1", localAssignmentStatement));
		CHECK(test_phrase_parser("local x = 42 + 3.14", localAssignmentStatement));
		CHECK(test_phrase_parser("local x, y = 'hello', 'World'", localAssignmentStatement));

		SUBCASE("one variable")
		{
			ast::LocalAssignmentStatement las;
			REQUIRE(test_phrase_parser("local x", localAssignmentStatement, las));
			REQUIRE(las.variables.size() == 1);
			CHECK(las.variables[0] == "x");
			CHECK(las.expressions.is_initialized() == false);
		}

		SUBCASE("two variables")
		{
			ast::LocalAssignmentStatement las;
			REQUIRE(test_phrase_parser("local x, y", localAssignmentStatement, las));
			REQUIRE(las.variables.size() == 2);
			CHECK(las.variables[0] == "x");
			CHECK(las.variables[1] == "y");
			CHECK(las.expressions.is_initialized() == false);
		}

		SUBCASE("two variables assigned")
		{
			ast::LocalAssignmentStatement las;
			REQUIRE(test_phrase_parser("local x, y = 42, 'hello'", localAssignmentStatement, las));
			REQUIRE(las.variables.size() == 2);
			CHECK(las.variables[0] == "x");
			CHECK(las.variables[1] == "y");

			REQUIRE(las.expressions.is_initialized());
			REQUIRE(las.expressions->size() == 2);
			const auto& exp1 = (*las.expressions)[0];
			REQUIRE(exp1.operand.isNumeral());
			REQUIRE(exp1.operand.asNumeral().isInt());
			REQUIRE(exp1.operand.asNumeral().asInt() == 42);

			const auto& exp2 = (*las.expressions)[1];
			REQUIRE(exp2.operand.isLiteral());
			CHECK(exp2.operand.asLiteral().value == "hello");
		}
	}

	TEST_CASE("label statement")
	{
		CHECK(test_phrase_parser("::test::", labelStatement));
		CHECK(test_phrase_parser(":: test ::", labelStatement));

		CHECK_FALSE(test_phrase_parser("::test:", labelStatement));
		CHECK_FALSE(test_phrase_parser(":: test 123 ::", labelStatement));
		CHECK_FALSE(test_phrase_parser("::::", labelStatement));

		SUBCASE("name")
		{
			ast::LabelStatement ls;
			REQUIRE(test_phrase_parser("::test::", labelStatement, ls));
			CHECK(ls.name == "test");
		}
	}

	TEST_CASE("goto statement")
	{
		CHECK(test_phrase_parser("goto test", gotoStatement));
		CHECK(test_phrase_parser("goto test123", gotoStatement));

		CHECK_FALSE(test_phrase_parser("goto 123", gotoStatement));

		SUBCASE("name")
		{
			ast::GotoStatement gs;
			REQUIRE(test_phrase_parser("goto test", gotoStatement, gs));
			CHECK(gs.label == "test");
		}
	}

	TEST_CASE("break statement")
	{
		CHECK(test_phrase_parser("break", breakStatement));
	}

	TEST_CASE("do statement")
	{
		CHECK(test_phrase_parser("do x = 1; print(x); end", doStatement));
	}

	TEST_CASE("while statement")
	{
		CHECK(test_phrase_parser("while x < 10 do x = x * 2 end", whileStatement));
	}

	TEST_CASE("repeat statement")
	{
		CHECK(test_phrase_parser("repeat x = x * 2 until x > 10", repeatStatement));
	}

	TEST_CASE("if statement")
	{
		CHECK(test_phrase_parser("if x < 10 then x = x * 2 end", ifThenElseStatement));
		CHECK(test_phrase_parser("if x < 10 then print(x) else print(x / 2) end", ifThenElseStatement));
		CHECK(test_phrase_parser("if x < 10 then print(x) elseif x < 20 then print(x / 2) else print(x / 4) end", ifThenElseStatement));

		SUBCASE("only if")
		{
			ast::IfThenElseStatement s;
			REQUIRE(test_phrase_parser("if x < 10 then print(x) end", ifThenElseStatement, s));
			CHECK(s.rest.empty());
			CHECK(s.elseBlock.is_initialized() == false);
		}

		SUBCASE("two elseif")
		{
			ast::IfThenElseStatement s;
			REQUIRE(test_phrase_parser("if x < 10 then print(x) "
									   "elseif x < 20 then print(x / 2) "
									   "elseif x < 30 then print(x / 3) "
									   " end",
									   ifThenElseStatement, s));
			CHECK(s.rest.size() == 2);
			CHECK(s.elseBlock.is_initialized() == false);
		}

		SUBCASE("with else")
		{
			ast::IfThenElseStatement s;
			REQUIRE(test_phrase_parser("if x < 10 then print(x) "
									   "elseif x < 20 then print(x / 2) "
									   "elseif x < 30 then print(x / 3) "
									   "else print(x / 4) end",
									   ifThenElseStatement, s));
			CHECK(s.rest.size() == 2);
			CHECK(s.elseBlock.is_initialized() == true);
		}
	}

	TEST_CASE("numerical for statement")
	{
		CHECK(test_phrase_parser("for x = 1, 10 do print(x) end", numericalForStatement));
		CHECK(test_phrase_parser("for x = 1, 10, 2 do print(x) end", numericalForStatement));

		SUBCASE("no step")
		{
			ast::NumericalForStatement s;
			REQUIRE(test_phrase_parser("for x = 1, 10 do print(x) end", numericalForStatement, s));
			CHECK(s.variable == "x");
			REQUIRE(s.first.operand.isNumeral());
			REQUIRE(s.first.operand.asNumeral().isInt());
			CHECK(s.first.operand.asNumeral().asInt() == 1);
			REQUIRE(s.last.operand.isNumeral());
			REQUIRE(s.last.operand.asNumeral().isInt());
			REQUIRE(s.last.operand.asNumeral().asInt() == 10);
			CHECK(s.step.is_initialized() == false);
		}

		SUBCASE("with step")
		{
			ast::NumericalForStatement s;
			REQUIRE(test_phrase_parser("for x = 1, 10, 2 do print(x) end", numericalForStatement, s));
			CHECK(s.variable == "x");
			REQUIRE(s.first.operand.isNumeral());
			REQUIRE(s.first.operand.asNumeral().isInt());
			REQUIRE(s.first.operand.asNumeral().asInt() == 1);
			REQUIRE(s.last.operand.isNumeral());
			REQUIRE(s.last.operand.asNumeral().isInt());
			REQUIRE(s.last.operand.asNumeral().asInt() == 10);
			REQUIRE(s.step.is_initialized());
			REQUIRE(s.step->operand.isNumeral());
			REQUIRE(s.step->operand.asNumeral().isInt());
			REQUIRE(s.step->operand.asNumeral().asInt() == 2);
		}
	}

	TEST_CASE("generic for statement")
	{
		CHECK(test_phrase_parser("for x in iter() do print(x) end", genericForStatement));
		CHECK(test_phrase_parser("for x, y in iterX(), iterY() do print(x) end", genericForStatement));

		SUBCASE("one name")
		{
			ast::GenericForStatement s;
			REQUIRE(test_phrase_parser("for x in iter() do print(x) end", genericForStatement, s));
			REQUIRE(s.variables.size() == 1);
			CHECK(s.variables[0] == "x");
			CHECK(s.expressions.size() == 1);
			REQUIRE(s.expressions[0].operand.get().type() == typeid(ast::f_PrefixExpression));
		}

		SUBCASE("two names")
		{
			ast::GenericForStatement s;
			REQUIRE(test_phrase_parser("for x, y in pairs(t) do print(x, y) end", genericForStatement, s));
			REQUIRE(s.variables.size() == 2);
			CHECK(s.variables[0] == "x");
			CHECK(s.variables[1] == "y");
			CHECK(s.expressions.size() == 1);
			REQUIRE(s.expressions[0].operand.get().type() == typeid(ast::f_PrefixExpression));
		}
	}

	TEST_CASE("function declaration statement")
	{
		CHECK(test_phrase_parser("function test() return 42 end", functionDeclarationStatement));
		CHECK(test_phrase_parser("function test(x, y) return 42 end", functionDeclarationStatement));
		CHECK(test_phrase_parser("function a.b:c(d) return d + 42 end", functionDeclarationStatement));

		SUBCASE("simple name")
		{
			ast::FunctionDeclarationStatement s;
			REQUIRE(test_phrase_parser("function test() return 42 end", functionDeclarationStatement, s));
			CHECK(s.name.start == "test");
			CHECK(s.name.rest.empty());
			CHECK(s.name.member.is_initialized() == false);
			CHECK(s.body.parameters.is_initialized() == false);
		}

		SUBCASE("complex name")
		{
			ast::FunctionDeclarationStatement s;
			REQUIRE(test_phrase_parser("function a.b:c(d) return d + 42 end", functionDeclarationStatement, s));
			CHECK(s.name.start == "a");
			REQUIRE(s.name.rest.size() == 1);
			CHECK(s.name.rest[0] == "b");
			REQUIRE(s.name.member.is_initialized());
			CHECK(s.name.member->name == "c");
			REQUIRE(s.body.parameters.is_initialized());
			CHECK(s.body.parameters->parameters.size() == 1);
		}
	}

	TEST_CASE("local function declaration statement")
	{
		CHECK(test_phrase_parser("local function test() return 42 end", localFunctionDeclarationStatement));
		CHECK(test_phrase_parser("local function test(x, y) return 42 end", localFunctionDeclarationStatement));
		CHECK_FALSE(test_phrase_parser("local function a.b:c(d) return d + 42 end", localFunctionDeclarationStatement));
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

		SUBCASE("no expression")
		{
			ast::ReturnStatement rs;
			CHECK(test_phrase_parser("return", returnStatement, rs));
			CHECK(rs.expressions.empty());
		}

		SUBCASE("one expression")
		{
			ast::ReturnStatement rs;
			CHECK(test_phrase_parser("return 42", returnStatement, rs));
			REQUIRE(rs.expressions.size() == 1);
			const auto& exp = rs.expressions[0];
			REQUIRE(exp.operand.isNumeral());
			REQUIRE(exp.operand.asNumeral().isInt());
			REQUIRE(exp.operand.asNumeral().asInt() == 42);
		}

		SUBCASE("two expressions")
		{
			ast::ReturnStatement rs;
			CHECK(test_phrase_parser("return 42, 'hello'", returnStatement, rs));
			REQUIRE(rs.expressions.size() == 2);
			const auto& exp1 = rs.expressions[0];
			REQUIRE(exp1.operand.isNumeral());
			REQUIRE(exp1.operand.asNumeral().isInt());
			REQUIRE(exp1.operand.asNumeral().asInt() == 42);
			const auto& exp2 = rs.expressions[1];
			REQUIRE(exp2.operand.isLiteral());
			CHECK(exp2.operand.asLiteral().value == "hello");
		}
	}

	template <class T>
	bool testStatementType(std::string_view str)
	{
		ast::Statement s;
		if (!test_phrase_parser(str, statement, s))
			return false;

		CHECK(s.get().type() == typeid(T));
		return s.get().type() == typeid(T);
	}

	TEST_CASE("statement")
	{
		CHECK(testStatementType<ast::EmptyStatement>(";"));
		CHECK(testStatementType<ast::AssignmentStatement>("x = 2"));
		CHECK(testStatementType<ast::AssignmentStatement>("a, b = 2.34, 42"));
		CHECK(testStatementType<ast::FunctionCall>("a(b)"));
		CHECK(testStatementType<ast::FunctionCall>("a:b(c, 42)"));
		CHECK(testStatementType<ast::LabelStatement>("::test::"));
		CHECK(testStatementType<ast::BreakStatement>("break"));
		CHECK(testStatementType<ast::GotoStatement>("goto test"));
		CHECK(testStatementType<ast::DoStatement>("do x = x+1 end"));
		CHECK(testStatementType<ast::WhileStatement>("while x < 10 do x = x + 1 end"));
		CHECK(testStatementType<ast::RepeatStatement>("repeat x = x + 1 until x > 10"));
		CHECK(testStatementType<ast::IfThenElseStatement>("if x < 10 then x = x + 1 else x = x - 1 end"));
		CHECK(testStatementType<ast::IfThenElseStatement>("if x < 10 then x = x + 1 elseif x > 20 then x = x - 1 end"));
		CHECK(testStatementType<ast::NumericalForStatement>("for x = 0, 10 do print(x) end"));
		CHECK(testStatementType<ast::GenericForStatement>("for x in 1, 2, 3 do print(x) end"));
		CHECK(testStatementType<ast::FunctionDeclarationStatement>("function test() return 42 end"));
		CHECK(testStatementType<ast::LocalFunctionDeclarationStatement>("local function test() return 42 end"));
		CHECK(testStatementType<ast::LocalAssignmentStatement>("local x = 42"));
		CHECK(testStatementType<ast::LocalAssignmentStatement>("local a, b = 2.34, 42"));

		CHECK_FALSE(test_phrase_parser("test", statement));
	}

	TEST_CASE("block")
	{
		CHECK(test_phrase_parser("local x = 42; return x", block));

		SUBCASE("no return")
		{
			ast::Block b;
			CHECK(test_phrase_parser("x = 2", block, b));
			CHECK(b.statements.size() == 1);
			CHECK(b.returnStatement.is_initialized() == false);
		}

		SUBCASE("with return")
		{
			ast::Block b;
			CHECK(test_phrase_parser("local x = 2\n print(x) return x + 42", block, b));
			CHECK(b.statements.size() == 2);
			CHECK(b.returnStatement.is_initialized() == true);
		}
	}
} // namespace lac::parser
