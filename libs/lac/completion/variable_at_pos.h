#pragma once

#include <lac/parser/ast.h>

namespace lac::comp
{
	std::string_view extractVariableAtPos(std::string_view view, size_t pos = std::string_view::npos);
	boost::optional<ast::VariableOrFunction> parseVariableAtPos(std::string_view view, size_t pos = std::string_view::npos);
} // namespace lac::comp
