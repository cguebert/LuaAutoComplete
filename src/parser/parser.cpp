#include <parser/chunk.h>
#include <parser/parser.h>
#include <parser/positions.h>

namespace lac::parser
{
	ParseBlockResults::ParseBlockResults(std::string_view view)
		: positions(view.begin(), view.end())
	{
	}

	ParseBlockResults parseBlock(std::string_view view, bool registerPositions)
	{
		ParseBlockResults res{view};

		auto f = view.begin();
		const auto l = view.end();
		if (registerPositions)
		{
			const auto parser = boost::spirit::x3::with<lac::pos::position_tag>(std::ref(res.positions))[chunkRule()];
			const auto skipper = boost::spirit::x3::with<lac::pos::position_tag>(std::ref(res.positions))[skipperRule()];
			res.parsed = boost::spirit::x3::phrase_parse(f, l, parser, skipper, res.block) && f == l;
		}
		else
		{
			res.parsed = boost::spirit::x3::phrase_parse(f, l, chunkRule(), skipperRule(), res.block) && f == l;
		}

		return res;
	}

	ParseVariableResults parseVariable(std::string_view view)
	{
		auto f = view.begin();
		const auto l = view.end();
		ParseVariableResults res;
		res.parsed = boost::spirit::x3::phrase_parse(f, l, variableOrFunctionRule(), skipperRule(), res.variable) && f == l;
		return res;
	}
} // namespace lac::parser
