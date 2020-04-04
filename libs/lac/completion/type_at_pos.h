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
	LAC_API an::TypeInfo getTypeAtPos(std::string_view view, size_t pos);
	LAC_API an::TypeInfo getTypeAtPos(const an::Scope& scope, std::string_view view, size_t pos);
	LAC_API an::TypeInfo getTypeAtPos(const an::Scope& scope, const ast::VariableOrFunction& var);
} // namespace lac::comp
