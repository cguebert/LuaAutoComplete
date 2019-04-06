#pragma once

namespace lac
{
	namespace ast
	{
		struct Block;
	}
	namespace an
	{
		class Scope;

		void analyseBlock(Scope& scope, const ast::Block& block);
	} // namespace an
} // namespace lac
