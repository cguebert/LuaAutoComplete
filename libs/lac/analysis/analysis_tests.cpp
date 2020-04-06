#include <lac/parser/chunk.h>
#include <lac/analysis/scope.h>
#include <lac/analysis/visitor.h>
#include <lac/analysis/user_defined.h>

#include <lac/helper/test_utils.h>

namespace lac
{
	using helper::test_phrase_parser;

	namespace an
	{
		doctest::String toString(const Type& value)
		{
			return std::to_string(static_cast<int>(value)).c_str();
		}

		void test_expression_type(const std::string& input, Type type)
		{
			ast::Block block;
			auto statement = "local x = " + input;
			REQUIRE(test_phrase_parser(statement, parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			CHECK(scope.getVariableType("x").type == type);
		}

#define EXPRESSION_TYPE(exp, type) \
	SUBCASE(exp) { test_expression_type(exp, type); }

		TEST_CASE("Expression type")
		{
			// One operand
			EXPRESSION_TYPE("nil", Type::nil);
			EXPRESSION_TYPE("true", Type::boolean);
			EXPRESSION_TYPE("42", Type::number);
			EXPRESSION_TYPE("'test'", Type::string);
			EXPRESSION_TYPE("{x=1, 2}", Type::table);
			EXPRESSION_TYPE("function () end", Type::function);

			// Logical operators
			EXPRESSION_TYPE("not {}", Type::boolean);
			EXPRESSION_TYPE("1 < 2", Type::boolean);
			EXPRESSION_TYPE("'a' < 'b'", Type::boolean);
			EXPRESSION_TYPE("1 == {}", Type::boolean);

			// Binary operations
			EXPRESSION_TYPE("-3", Type::number);
			EXPRESSION_TYPE("42 + 3.14", Type::number);
			EXPRESSION_TYPE("42 + 3 / 2 * 5 // 4", Type::number);
			EXPRESSION_TYPE("'hello' .. ' world'", Type::string);
			EXPRESSION_TYPE("1 or 42", Type::number);

			// Unknown types (depend on the values)
			EXPRESSION_TYPE("1 or 'test'", Type::unknown);
			EXPRESSION_TYPE("'hello' and {1, 2}", Type::unknown);

			// Conversions
			EXPRESSION_TYPE("42 + '3.14'", Type::number);
			EXPRESSION_TYPE("'test' .. 2", Type::string);

			// Errors
			EXPRESSION_TYPE("-{}", Type::error);
			EXPRESSION_TYPE("#42", Type::error);
			EXPRESSION_TYPE("1 + {}", Type::error);
			EXPRESSION_TYPE("1 + function() end", Type::error);
			EXPRESSION_TYPE("2 > 'a'", Type::error);
		}

		TEST_CASE("Simple assignment")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("x = 42", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("x");
			CHECK(info.type == Type::number);
		}

		TEST_CASE("Multiple assignments")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("x, y, z = 42, true, 'test'", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			CHECK(scope.getVariableType("x").type == Type::number);
			CHECK(scope.getVariableType("y").type == Type::boolean);
			CHECK(scope.getVariableType("z").type == Type::string);
		}

		TEST_CASE("Function assignment")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("local x = function(x, y, z) end", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("x");
			REQUIRE(info.type == Type::function);
			const auto& fd = info.function;
			REQUIRE(fd.parameters.size() == 3);
			CHECK(fd.parameters[0].name() == "x");
			CHECK(fd.parameters[0].type().type == Type::unknown);
			CHECK(fd.parameters[1].name() == "y");
			CHECK(fd.parameters[2].name() == "z");
		}

		TEST_CASE("Function definition")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("function func(x, y) end", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getFunctionType("func");
			REQUIRE(info.type == Type::function);
			const auto& fd = info.function;
			REQUIRE(fd.parameters.size() == 2);
			CHECK(fd.parameters[0].name() == "x");
			CHECK(fd.parameters[0].type().type == Type::unknown);
			CHECK(fd.parameters[1].name() == "y");
		}

		TEST_CASE("Local function definition")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("local function func(x, y) end", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getFunctionType("func");
			REQUIRE(info.type == Type::function);
			const auto& fd = info.function;
			REQUIRE(fd.parameters.size() == 2);
			CHECK(fd.parameters[0].name() == "x");
			CHECK(fd.parameters[0].type().type == Type::unknown);
			CHECK(fd.parameters[1].name() == "y");
		}

		TEST_CASE("Table constructor by assignement")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("t = {x=42, name='foo', test=false}", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("t");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 3);

			REQUIRE(info.members.count("x"));
			CHECK(info.members.at("x").type == Type::number);

			REQUIRE(info.members.count("name"));
			CHECK(info.members.at("name").type == Type::string);

			REQUIRE(info.members.count("test"));
			CHECK(info.members.at("test").type == Type::boolean);
		}

		TEST_CASE("Table constructor expression only")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("t = {42, 'test', false}", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("t");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 3);

			REQUIRE(info.members.count("1"));
			CHECK(info.members.at("1").type == Type::number);

			REQUIRE(info.members.count("2"));
			CHECK(info.members.at("2").type == Type::string);

			REQUIRE(info.members.count("3"));
			CHECK(info.members.at("3").type == Type::boolean);
		}

		TEST_CASE("Table constructor varied")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("t = {1, text='foo', false}", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("t");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 3);

			REQUIRE(info.members.count("1"));
			CHECK(info.members.at("1").type == Type::number);

			REQUIRE(info.members.count("2"));
			CHECK(info.members.at("2").type == Type::boolean);

			REQUIRE(info.members.count("text"));
			CHECK(info.members.at("text").type == Type::string);

			CHECK_FALSE(info.members.count("3"));
		}

		TEST_CASE("Table member assignment")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("t = {}; t.x=42; t.str='foo'; t.test=true", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("t");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 3);

			REQUIRE(info.members.count("x"));
			CHECK(info.members.at("x").type == Type::number);

			REQUIRE(info.members.count("str"));
			CHECK(info.members.at("str").type == Type::string);

			REQUIRE(info.members.count("test"));
			CHECK(info.members.at("test").type == Type::boolean);
		}

		TEST_CASE("Table member hierarchy")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("a={}; a.b={}; a.b.v=42; a.b.c={}; a.b.c.t=true", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("a");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 1);

			CHECK(info.member("b").type == Type::table);
			CHECK(info.member("b").member("v").type == Type::number);
			CHECK(info.member("b").member("c").type == Type::table);
			CHECK(info.member("b").member("c").member("t").type == Type::boolean);
		}

		TEST_SUITE_BEGIN("User defined");

		TEST_CASE("Variables")
		{
			UserDefined user;
			user.addVariable("x", Type::number);
			user.addVariable("y", Type::boolean);

			Scope scope;
			scope.setUserDefined(&user);

			CHECK(scope.getVariableType("x").type == Type::number);
			CHECK(scope.getVariableType("y").type == Type::boolean);
		}

		TEST_CASE("Functions")
		{
			UserDefined user;
			user.addFreeFunction("func", {{{"x", Type::number},
										   {"y"}},
										  {Type::number}});

			Scope scope;
			scope.setUserDefined(&user);

			const auto func = scope.getFunctionType("func");
			REQUIRE(func.type == Type::function);
			REQUIRE(func.function.results.size() == 1);
			CHECK(func.function.results[0].type == Type::number);
			REQUIRE(func.function.parameters.size() == 2);
			CHECK(func.function.parameters[0].name() == "x");
			CHECK(func.function.parameters[0].type().type == Type::number);
			CHECK(func.function.parameters[1].name() == "y");
			CHECK(func.function.parameters[1].type().type == Type::unknown);
		}

		TEST_CASE("Type")
		{
			UserDefined user;

			TypeInfo complexType = Type::table;
			complexType.name = "complex";
			complexType.members["real"] = Type::number;
			complexType.members["imag"] = Type::number;
			user.addType(complexType);

			user.addFreeFunction("createComplex", {{{"real", Type::number},
													{"imag", Type::number}},
												   {TypeInfo::fromTypeName("complex")}});

			user.addScriptInput("run", {{{"num", Type::number},
										 {"name", Type::string}}});

			Scope parentScope;
			parentScope.setUserDefined(&user);

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = createComplex(0.7, 1.0)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::userdata);
				CHECK(info.name == "complex");
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = createComplex(0.7, 1.0).real", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::number);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = createComplex(0.7, 1.0); local y = x.imag", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto infoX = scope.getVariableType("x");
				CHECK(infoX.type == Type::userdata);
				CHECK(infoX.name == "complex");
				const auto type = scope.getUserType("complex");
				CHECK(type.members.size() == 2);
				CHECK(type.member("real").type == Type::number);
				CHECK(type.member("imag").type == Type::number);
				CHECK(type.member("xxx").type == Type::nil);
				const auto infoY = scope.getVariableType("y");
				CHECK(infoY.type == Type::number);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("function run(i, n) print(n) end", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				REQUIRE(scope.children().size() == 1);
				const auto& funcScope = scope.children().front();
				const auto infoI = funcScope.getVariableType("i");
				CHECK(infoI.type == Type::number);
				const auto infoN = funcScope.getVariableType("n");
				CHECK(infoN.type == Type::string);
			}
		}

		TEST_SUITE_END();
	} // namespace an
} // namespace lac
