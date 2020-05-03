#include <lac/parser/chunk.h>
#include <lac/parser/parser.h>
#include <lac/parser/positions.h>

#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

namespace lac::parser
{
	ParseBlockResults::ParseBlockResults(std::string_view view)
		: positions(view.begin(), view.end())
	{
	}

	ParseBlockResults parseBlock(std::string_view view, bool registerPositions)
	{
		ParseBlockResults res{view};
		if (view.empty())
			return res;

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

		res.lastParsedPosition = f - view.begin();
		return res;
	}

	ParseVariableResults parseVariable(std::string_view view)
	{
		ParseVariableResults res;
		if (view.empty())
			return res;

		auto f = view.begin();
		const auto l = view.end();
		res.parsed = boost::spirit::x3::phrase_parse(f, l, variableOrFunctionRule(), skipperRule(), res.variable) && f == l;
		return res;
	}
} // namespace lac::parser
