#pragma once

#include <lac/parser/ast.h>
#include <lac/parser/positions.h>
#include <lac/lac_api.h>

namespace lac::parser
{
	struct LAC_API ParseBlockResults
	{
		ParseBlockResults(std::string_view view);

		bool parsed = false;
		ast::Block block;
		pos::Positions<std::string_view::const_iterator> positions;
	};

	// These skip comments and spaces
	LAC_API ParseBlockResults parseBlock(std::string_view view, bool registerPositions = true);

	struct LAC_API ParseVariableResults
	{
		bool parsed = false;
		ast::VariableOrFunction variable;
	};

	LAC_API ParseVariableResults parseVariable(std::string_view view);
} // namespace lac::parser
