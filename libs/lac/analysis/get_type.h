#pragma once

#include <lac/analysis/type_info.h>

namespace lac::ast
{
	struct Expression;
	struct FunctionBody;
}

namespace lac::an
{
	class Scope;

	TypeInfo getType(const Scope& scope, const ast::Expression& e);
	TypeInfo getType(const Scope& scope, const ast::FunctionBody& f);
} // namespace lac::an
