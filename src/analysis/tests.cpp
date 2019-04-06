#include <parser/chunk.h>
#include <analysis/scope.h>
#include <analysis/visitor.h>

#include <helper/test_utils.h>

namespace lac
{
	using helper::test_phrase_parser;

	namespace an
	{
		TEST_CASE("Scope")
		{
			auto chunk = chunkRule();

			ast::Block block;
			REQUIRE(test_phrase_parser("local x = 42", chunk, block));

			Scope scope;
			analyseBlock(scope, block);
			CHECK(scope.getVariableType("x").type == Type::number);
		}
	} // namespace an
} // namespace lac
