#include <parser/ast_adapted.h>
#include <parser/chunk.h>
#include <parser/config.h>

#include <helper/test_utils.h>

namespace
{
	template <class P, class A>
	bool phrase_parser_elements(std::string_view input, const P& p, A& arg, lac::pos::Elements& elts)
	{
		auto f = input.begin();
		const auto l = input.end();
		lac::pos::Positions positions{f, l};
		const auto parser = boost::spirit::x3::with<lac::pos::position_tag>(std::ref(positions))[p];
		if (boost::spirit::x3::phrase_parse(f, l, parser, boost::spirit::x3::ascii::space, arg) && f == l)
		{
			elts = positions.elements();
			return true;
		}
		return false;
	}
} // namespace

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

	TEST_CASE("Elements")
	{
		auto chunk = chunkRule();

		ast::Block block;
		pos::Elements elements;
		REQUIRE(phrase_parser_elements("testVar = 'hello' .. 42", chunk, block, elements));
		REQUIRE(elements.size() == 3);

		const auto var = elements[0];
		CHECK(var.type == ast::ElementType::variable);
		CHECK(var.begin == 0);
		CHECK(var.end == 7);

		const auto str = elements[1];
		CHECK(str.type == ast::ElementType::literal_string);
		CHECK(str.begin == 10);
		CHECK(str.end == 17);

		const auto num = elements[2];
		CHECK(num.type == ast::ElementType::numeral);
		CHECK(num.begin == 21);
		CHECK(num.end == 23);
	}
} // namespace lac
