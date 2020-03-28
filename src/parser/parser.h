#pragma once

#include <parser/ast.h>
#include <parser/positions.h>

namespace lac::parser
{
	// These skip comments and spaces
	bool parseString(std::string_view view, pos::Positions<std::string_view::const_iterator>& positions, ast::Block& block);
	bool parseString(std::string_view view, ast::Block& block);

	bool parseString(std::string_view view, ast::VariableOrFunction& variable);
} // namespace lac
