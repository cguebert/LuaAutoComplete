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
		void analyseBlock(Scope& scope, const ast::Block& block);
		Scope analyseBlock(const ast::Block& block, Scope* parentScope = nullptr);
	} // namespace an
} // namespace lac
