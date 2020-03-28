#include <parser/config.h>
#include <parser/parser.h>
#include <parser/positions.h>

namespace lac::parser
{
	bool parseString(std::string_view view, pos::Positions<std::string_view::const_iterator>& positions, ast::Block& block)
	{
		const auto parser = boost::spirit::x3::with<lac::pos::position_tag>(std::ref(positions))[chunkRule()];
		const auto skipper = boost::spirit::x3::with<lac::pos::position_tag>(std::ref(positions))[skipperRule()];

		auto f = view.begin();
		const auto l = view.end();
		return boost::spirit::x3::phrase_parse(f, l, parser, skipper, block) && f == l;
	}

	bool parseString(std::string_view view, ast::Block& block)
	{
		auto f = view.begin();
		const auto l = view.end();
		return boost::spirit::x3::phrase_parse(f, l, chunkRule(), skipperRule(), block) && f == l;
	}

	bool parseString(std::string_view view, ast::VariableOrFunction& variable)
	{
		auto f = view.begin();
		const auto l = view.end();
		return boost::spirit::x3::phrase_parse(f, l, variableOrFunctionRule(), skipperRule(), variable) && f == l;
	}
} // namespace lac::parser
