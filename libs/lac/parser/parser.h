#pragma once

#include <lac/parser/ast.h>
#include <lac/parser/positions.h>
#include <lac/core_api.h>

namespace lac::parser
{
	struct CORE_API ParseBlockResults
	{
		ParseBlockResults(std::string_view view);

		bool parsed = false;
		ast::Block block;
		pos::Positions<std::string_view::const_iterator> positions;
		size_t lastParsedPosition = 0;
	};

	// These skip comments and spaces
	CORE_API ParseBlockResults parseBlock(std::string_view view, bool registerPositions = true);

	struct CORE_API ParseVariableResults
	{
		bool parsed = false;
		ast::VariableOrFunction variable;
	};

	CORE_API ParseVariableResults parseVariable(std::string_view view);
} // namespace lac::parser
