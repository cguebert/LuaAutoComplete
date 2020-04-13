#pragma once

#include <lac/analysis/type_info.h>

namespace lac::ast
{
	struct VariableOrFunction;
}

namespace lac::an
{
	class Scope;
}

namespace lac::comp
{
	// Return the type information about the variable under the cursor
	CORE_API an::TypeInfo getTypeAtPos(std::string_view view, size_t pos = std::string_view::npos);
	CORE_API an::TypeInfo getTypeAtPos(const an::Scope& rootScope, std::string_view view, size_t pos = std::string_view::npos);

	// Return the type information about the given variable
	CORE_API an::TypeInfo getVariableType(const an::Scope& localScope, const ast::VariableOrFunction& var);

	// Return the name of the type and the chain of members of the variable under the cursor
	CORE_API std::vector<std::string> getTypeHierarchyAtPos(const an::Scope& rootScope, std::string_view view, size_t pos = std::string_view::npos);
} // namespace lac::comp
