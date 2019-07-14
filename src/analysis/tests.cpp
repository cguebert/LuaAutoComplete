#include <parser/chunk.h>
#include <analysis/scope.h>
#include <analysis/visitor.h>
#include <analysis/user_defined.h>

#include <helper/test_utils.h>

namespace lac
{
	using helper::test_phrase_parser;

	namespace an
	{
		void test_expression_type(const std::string& input, Type type)
		{
			ast::Block block;
			auto statement = "local x = " + input;
			REQUIRE(test_phrase_parser(statement, chunkRule(), block));

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

		TEST_CASE("Function assignment")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("local x = function(x, y, z) end", chunkRule(), block));

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

		TEST_CASE("Local function definition")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("local function func(x, y) end", chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getFunctionType("func");
			REQUIRE(info.type == Type::function);
			const auto& fd = info.function;
			REQUIRE(fd.parameters.size() == 2);
			CHECK(fd.parameters[0].name() == "x");
			CHECK(fd.parameters[0].type().type == Type::unknown);
			CHECK(fd.parameters[1].name() == "y");
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

			UserType complexType;
			complexType.name = "complex";
			complexType.variables["real"] = Type::number;
			complexType.variables["imag"] = Type::number;
			user.addType(complexType);

			FunctionInfo createComplex;
			createComplex.parameters = {{"real", Type::number},
										{"imag", Type::number}};
			createComplex.results.emplace_back(TypeInfo::fromTypeName("complex"));
			user.addFreeFunction("createComplex", createComplex);

			Scope parentScope;
			parentScope.setUserDefined(&user);

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = createComplex(0.7, 1.0)", chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::userdata);
				CHECK(info.userType == "complex");
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = createComplex(0.7, 1.0).real", chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::number);
			}
		}

		TEST_SUITE_END();
	} // namespace an
} // namespace lac
