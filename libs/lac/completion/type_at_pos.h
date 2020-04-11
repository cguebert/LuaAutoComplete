#pragma once

#include <lac/analysis/type_info.h>

namespace lac::ast
{
	struct Variable;
	struct VariableOrFunction;
}

namespace lac::an
{
	class Scope;
}

namespace lac::comp
{
	LAC_API an::TypeInfo getTypeAtPos(std::string_view view, size_t pos = std::string_view::npos);
	LAC_API an::TypeInfo getTypeAtPos(const an::Scope& rootScope, std::string_view view, size_t pos = std::string_view::npos);

	LAC_API an::TypeInfo getVariableType(const an::Scope& localScope, const ast::Variable& var);
	LAC_API an::TypeInfo getVariableType(const an::Scope& localScope, const ast::VariableOrFunction& var);
} // namespace lac::comp
