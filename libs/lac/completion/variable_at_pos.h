#pragma once

#include <lac/parser/ast.h>

namespace lac::comp
{
	// Extract the text under the cursor if it can form a variable
	std::string_view extractVariableAtPos(std::string_view view, size_t pos = std::string_view::npos);

	// Extract and parse the variable under the cursor
	boost::optional<ast::VariableOrFunction> parseVariableAtPos(std::string_view view, size_t pos = std::string_view::npos);
} // namespace lac::comp
