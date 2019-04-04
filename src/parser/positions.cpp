#include <parser/ast_adapted.h>
#include <parser/chunk.h>
#include <parser/config.h>
#include <parser/positions.h>

#include <doctest/doctest.h>

namespace lac
{
	namespace x3 = boost::spirit::x3;
	template <class P, class... Args>
	bool test_phrase_parser(std::string_view input, const P& p, Args&... args)
	{
		auto f = input.begin();
		const auto l = input.end();
		return x3::phrase_parse(f, l, p, x3::ascii::space, args...) && f == l;
	}

	TEST_CASE("Positions")
	{
		const std::string input = "testVar = 'hello' .. 42";
		const std::string_view view = input;
		pos::Positions positions{view.begin(), view.end()};
		auto chunk = chunkRule();
		const auto parser = x3::with<pos::position_tag>(std::ref(positions))[chunk];

		ast::Block block;
		REQUIRE(test_phrase_parser(input, parser, block));
		REQUIRE(block.statements.size() == 1);
		REQUIRE(block.statements.front().get().type() == typeid(ast::AssignmentStatement));
		auto assignment = boost::get<ast::AssignmentStatement>(block.statements.front());
		REQUIRE(assignment.variables.size() == 1);
		auto var = assignment.variables.front();
		CHECK(var.begin == 0);
		CHECK(var.end == 7);
	}
} // namespace lac
