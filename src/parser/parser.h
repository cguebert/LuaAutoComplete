#pragma once

#include <parser/ast.h>
#include <parser/positions.h>

namespace lac::parser
{
	struct ParseBlockResults
	{
		ParseBlockResults(std::string_view view);

		bool parsed = false;
		ast::Block block;
		pos::Positions<std::string_view::const_iterator> positions;
	};

	// These skip comments and spaces
	ParseBlockResults parseBlock(std::string_view view, bool registerPositions = true);

	struct ParseVariableResults
	{
		bool parsed = false;
		ast::VariableOrFunction variable;
	};

	ParseVariableResults parseVariable(std::string_view view);
} // namespace lac::parser
