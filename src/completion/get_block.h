#pragma once

#include <parser/chunk.h>

namespace lac::pos
{
	using Blocks = std::vector<const ast::Block*>;
	Blocks getChildren(const ast::Block& block);
	const ast::Block* getBlockAtPos(const ast::Block& root, size_t pos);
} // namespace lac::pos
