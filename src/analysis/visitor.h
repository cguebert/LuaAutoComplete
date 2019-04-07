#pragma once

#include <analysis/scope.h>

namespace lac
{
	namespace ast
	{
		struct Block;
	}
	namespace an
	{
		Scope analyseBlock(const ast::Block& block, Scope* parentScope = nullptr);
	} // namespace an
} // namespace lac
