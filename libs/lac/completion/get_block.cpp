#include <lac/completion/get_block.h>
#include <lac/analysis/scope.h>

#include <lac/helper/algorithm.h>

namespace lac::pos
{
	using namespace ast;

	class GetChildrenBlocks : public boost::static_visitor<void>
	{
	public:
		GetChildrenBlocks(Blocks& blocks)
			: m_blocks(blocks)
		{
		}

		void operator()(ast::ExpressionConstant) const
		{
			// Nothing to do here
		}

		void operator()(const ast::Numeral&) const
		{
			// Nothing to do here
		}

		void operator()(const ast::LiteralString&) const
		{
			// Nothing to do here
		}

		void operator()(const std::string&) const
		{
			// Nothing to do here
		}

		void operator()(const ast::UnaryOperation& uo) const
		{
			(*this)(uo.expression);
		}

		void operator()(const ast::BinaryOperation& bo) const
		{
			(*this)(bo.expression);
		}

		void operator()(const ast::FieldByExpression& f) const
		{
			(*this)(f.key);
			(*this)(f.value);
		}

		void operator()(const ast::FieldByAssignment& f) const
		{
			(*this)(f.value);
		}

		void operator()(const ast::Field& f) const
		{
			boost::apply_visitor(*this, f);
		}

		void operator()(const ast::TableConstructor& tc) const
		{
			if (tc.fields)
			{
				for (const auto& f : *tc.fields)
					(*this)(f);
			}
		}

		void operator()(const ast::Operand& op) const
		{
			boost::apply_visitor(*this, op);
		}

		void operator()(const ast::BracketedExpression be) const
		{
			(*this)(be.expression);
		}

		void operator()(const ast::TableIndexExpression tie) const
		{
			(*this)(tie.expression);
		}

		void operator()(const ast::TableIndexName&) const
		{
			// Nothing to do here
		}

		void operator()(const ast::EmptyArguments&) const
		{
			// Nothing to do here
		}

		void operator()(const ast::FunctionCallEnd& fce) const
		{
			boost::apply_visitor(*this, fce.arguments);
		}

		void operator()(const ast::PrefixExpression& pe) const
		{
			boost::apply_visitor(*this, pe.start);
			for (const auto& pp : pe.rest)
				boost::apply_visitor(*this, pp);
		}

		void operator()(const ast::VariableFunctionCall& vfc) const
		{
			(*this)(vfc.functionCall);
			boost::apply_visitor(*this, vfc.postVariable);
		}

		void operator()(const ast::Variable& v) const
		{
			boost::apply_visitor(*this, v.start);
			for (const auto& vp : v.rest)
				boost::apply_visitor(*this, vp);
		}

		void operator()(const ast::Expression& e) const
		{
			(*this)(e.operand);
			if (e.binaryOperation)
				(*this)(*e.binaryOperation);
		}

		void operator()(const ast::ExpressionsList& el) const
		{
			for (const auto& ex : el)
				(*this)(ex);
		}

		void operator()(const ast::ReturnStatement& rs) const
		{
			for (const auto& ex : rs.expressions)
				(*this)(ex);
		}

		void operator()(const ast::FunctionBody& fb) const
		{
			(*this)(fb.block);
		}

		void operator()(const ast::EmptyStatement&) const
		{
		}

		void operator()(const ast::AssignmentStatement& as) const
		{
			(*this)(as.expressions);
		}

		void operator()(const ast::FunctionCall&) const
		{
		}

		void operator()(const ast::LabelStatement&) const
		{
		}

		void operator()(const ast::GotoStatement&) const
		{
		}

		void operator()(const ast::BreakStatement&) const
		{
		}

		void operator()(const ast::DoStatement& ds) const
		{
			(*this)(ds.block);
		}

		void operator()(const ast::WhileStatement& ws) const
		{
			(*this)(ws.condition);
			(*this)(ws.block);
		}

		void operator()(const ast::RepeatStatement& rs) const
		{
			(*this)(rs.block);
			(*this)(rs.condition);
		}

		void operator()(const ast::IfStatement& s) const
		{
			(*this)(s.condition);
			(*this)(s.block);
		}

		void operator()(const ast::IfThenElseStatement& s) const
		{
			(*this)(s.first);
			for (const auto& es : s.rest)
				(*this)(es);
			if (s.elseBlock)
				(*this)(*s.elseBlock);
		}

		void operator()(const ast::NumericalForStatement& s) const
		{
			(*this)(s.first);
			(*this)(s.last);
			if (s.step)
				(*this)(*s.step);
			(*this)(s.block);
		}

		void operator()(const ast::GenericForStatement& s) const
		{
			(*this)(s.expressions);
			(*this)(s.block);
		}

		void operator()(const ast::FunctionDeclarationStatement& s) const
		{
			(*this)(s.body);
		}

		void operator()(const ast::LocalFunctionDeclarationStatement& s) const
		{
			(*this)(s.body);
		}

		void operator()(const ast::LocalAssignmentStatement& s) const
		{
			if (s.expressions)
				(*this)(*s.expressions);
		}

		void operator()(const Block& b) const
		{
			m_blocks.push_back(&b);
			for (const auto& s : b.statements)
				boost::apply_visitor(*this, s);
			if (b.returnStatement)
				(*this)(*b.returnStatement);
		}

	private:
		Blocks& m_blocks;
	};

	Blocks getChildren(const ast::Block& block)
	{
		Blocks blocks;
		GetChildrenBlocks{blocks}(block);
		return blocks;
	}

	const ast::Block* getBlockAtPos(const ast::Block& root, size_t pos)
	{
		if (root.begin > pos || root.end < pos)
			return nullptr;

		const auto children = getChildren(root);
		return getBlockAtPos(children, pos);
	}

	const ast::Block* getBlockAtPos(const Blocks& blocks, size_t pos)
	{
		helper::reverse reversed{blocks};
		const auto it = helper::find_if(reversed, [pos](const ast::Block* block) {
			return block->begin <= pos && block->end >= pos;
		});

		return it != reversed.end()
				   ? *it
				   : nullptr;
	}

	const an::Scope* getScopeAtPos(const an::Scope& scope, size_t pos)
	{
		const auto block = scope.block();
		if (!block || block->begin > pos || block->end < pos)
			return nullptr;

		for (const auto& child : scope.children())
		{
			const auto ptr = getScopeAtPos(child, pos);
			if (ptr)
				return ptr;
		}

		return &scope;
	}
} // namespace lac::pos
