#pragma once

#include <lac/parser/chunk.h>

namespace lac::an
{
	class Scope;
}

namespace lac::pos
{
	using Blocks = std::vector<const ast::Block*>;

	Blocks getChildren(const ast::Block& block);
	const ast::Block* getBlockAtPos(const ast::Block& root, size_t pos);
	const ast::Block* getBlockAtPos(const Blocks& blocks, size_t pos);

	const an::Scope* getScopeAtPos(const an::Scope& root, size_t pos);
} // namespace lac::pos
