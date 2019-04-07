#include <parser/chunk.h>
#include <analysis/scope.h>
#include <analysis/visitor.h>

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

		TEST_CASE("Function definition")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("local x = function(x, y, z) end", chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("x");
			REQUIRE(info.type == Type::function);
			REQUIRE(info.parameters.size() == 3);
			CHECK(info.parameters[0].name == "x");
			CHECK(info.parameters[0].type->type == Type::unknown);
			CHECK(info.parameters[1].name == "y");
			CHECK(info.parameters[2].name == "z");
		}
	} // namespace an
} // namespace lac
