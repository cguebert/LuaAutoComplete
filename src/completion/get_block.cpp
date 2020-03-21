#include <completion/get_block.h>

#include <helper/algorithm.h>

namespace lac::pos
{
	using namespace ast;

	void append(Blocks& container, const Blocks& elements)
	{
		container.insert(container.end(), elements.begin(), elements.end());
	}

	class GetChildrenBlocks : public boost::static_visitor<Blocks>
	{
	public:
		Blocks operator()(const ast::EmptyStatement&) const
		{
			return {};
		}

		Blocks operator()(const ast::AssignmentStatement&) const
		{
			return {};
		}

		Blocks operator()(const ast::FunctionCall&) const
		{
			return {};
		}

		Blocks operator()(const ast::LabelStatement&) const
		{
			return {};
		}

		Blocks operator()(const ast::GotoStatement&) const
		{
			return {};
		}

		Blocks operator()(const ast::BreakStatement&) const
		{
			return {};
		}

		Blocks operator()(const ast::DoStatement& ds) const
		{
			return (*this)(ds.block);
		}

		Blocks operator()(const ast::WhileStatement& ws) const
		{
			return (*this)(ws.block);
		}

		Blocks operator()(const ast::RepeatStatement& rs) const
		{
			return (*this)(rs.block);
		}

		Blocks operator()(const ast::IfThenElseStatement& s) const
		{
			Blocks blocks;
			append(blocks, (*this)(s.first.block));
			for (const auto& es : s.rest)
				append(blocks, (*this)(es.block));
			if (s.elseBlock)
				append(blocks, (*this)(*s.elseBlock));
			return blocks;
		}

		Blocks operator()(const ast::NumericalForStatement& s) const
		{
			return (*this)(s.block);
		}

		Blocks operator()(const ast::GenericForStatement& s) const
		{
			return (*this)(s.block);
		}

		Blocks operator()(const ast::FunctionDeclarationStatement& s) const
		{
			return (*this)(s.body.block);
		}

		Blocks operator()(const ast::LocalFunctionDeclarationStatement& s) const
		{
			return (*this)(s.body.block);
		}

		Blocks operator()(const ast::LocalAssignmentStatement&) const
		{
			return {};
		}

		Blocks operator()(const Block& b) const
		{
			Blocks blocks;
			blocks.push_back(&b);
			for (const auto& s : b.statements)
				append(blocks, boost::apply_visitor(*this, s));
			return blocks;
		}
	};

	Blocks getChildren(const ast::Block& block)
	{
		return GetChildrenBlocks{}(block);
	}

	const ast::Block* getBlockAtPos(const ast::Block& root, size_t pos)
	{
		if (root.begin > pos || root.end < pos)
			return nullptr;

		const auto children = getChildren(root);
		helper::reverse reversed{children};
		const auto it = helper::find_if(reversed, [pos](const ast::Block* block) {
			return block->begin <= pos && block->end >= pos;
		});

		return it != reversed.end()
				   ? *it
				   : nullptr;
	}
} // namespace lac::pos
