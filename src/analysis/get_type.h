#pragma once

#include <analysis/type_info.h>
#include <parser/ast.h>

namespace lac::an
{
	class Scope;

	TypeInfo getType(const Scope& scope, const ast::Expression& e);
} // namespace lac::an
