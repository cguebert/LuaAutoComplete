#include <parser/ast_adapted.h>
#include <parser/chunk.h>
#include <parser/config.h>

#include <helper/test_utils.h>

namespace lac
{
	using helper::test_phrase_parser;

	TEST_CASE("Positions")
	{
		auto chunk = chunkRule();

		ast::Block block;
		REQUIRE(test_phrase_parser("testVar = 'hello' .. 42", chunk, block));
		REQUIRE(block.statements.size() == 1);
		REQUIRE(block.statements.front().get().type() == typeid(ast::AssignmentStatement));
		auto assignment = boost::get<ast::AssignmentStatement>(block.statements.front());
		REQUIRE(assignment.variables.size() == 1);
		auto var = assignment.variables.front();
		CHECK(var.begin == 0);
		CHECK(var.end == 7);
	}
} // namespace lac
