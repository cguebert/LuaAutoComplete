#pragma once

#include <parser/ast.h>

namespace lac::pos
{
	std::string_view extractVariableAtPos(std::string_view view, size_t pos);
	boost::optional<ast::Variable> parseVariableAtPos(std::string_view view, size_t pos);
} // namespace lac::pos
