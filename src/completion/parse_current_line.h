#pragma once

#include <parser/ast.h>

namespace lac::comp
{
	std::string_view extractVariableAtPos(std::string_view view, size_t pos);
	boost::optional<ast::VariableOrFunction> parseVariableAtPos(std::string_view view, size_t pos);
} // namespace lac::comp
