#pragma once

#include <analysis/type_info.h>
#include <parser/ast.h>

namespace lac::an
{
	class Scope;

	TypeInfo getSubType(const Scope& scope, const TypeInfo& parentType, const ast::PostPrefix& pp);
	TypeInfo getSubType(const Scope& scope, const TypeInfo& parentType, const ast::VariablePostfix& vp);
	TypeInfo getSubType(const Scope& scope, const TypeInfo& parentType, const ast::FunctionCallEnd& fce);
} // namespace lac::an
