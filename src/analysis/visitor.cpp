#include <analysis/visitor.h>
#include <analysis/scope.h>
#include <analysis/get_type.h>

#include <parser/ast.h>

namespace lac::an
{
	class AnalysisVisitor : public boost::static_visitor<void>
	{
	public:
		AnalysisVisitor(Scope& scope)
			: m_scope(scope)
		{
		}

		void operator()(ast::ExpressionConstant ec) const
		{
		}

		void operator()(int v) const
		{
		}

		void operator()(double v) const
		{
		}

		void operator()(const std::string& str) const
		{
		}

		void operator()(const ast::UnaryOperation& uo) const
		{
		}

		void operator()(const ast::BinaryOperation& bo) const
		{
		}

		void operator()(const ast::Field& f) const
		{
		}

		void operator()(const ast::FieldsList& fl) const
		{
		}

		void operator()(const ast::TableConstructor& tc) const
		{
		}

		void operator()(const ast::FunctionBody& fb) const
		{
		}

		void operator()(const ast::Operand& operand) const
		{
		}

		void operator()(const ast::Expression& ex) const
		{
		}

		void operator()(const ast::BracketedExpression be) const
		{
		}

		void operator()(const ast::TableIndexExpression tie) const
		{
		}

		void operator()(const ast::TableIndexName& tin) const
		{
		}

		void operator()(const ast::ParametersList& pl) const
		{
		}

		void operator()(const ast::Arguments& arg) const
		{
		}

		void operator()(const ast::FunctionCallEnd& fce) const
		{
		}

		void operator()(const ast::PostPrefix& pp) const
		{
		}

		void operator()(const ast::PrefixExpression& pe) const
		{
		}

		void operator()(const ast::VariablePostfix& vp) const
		{
		}

		void operator()(const ast::VariableFunctionCall& vfc) const
		{
		}

		void operator()(const ast::Variable& v) const
		{
		}

		void operator()(const ast::FunctionCall& fn) const
		{
		}

		void operator()(const ast::ReturnStatement& rs) const
		{
		}

		void operator()(const ast::EmptyStatement&) const
		{
			// Nothing to do here
		}

		void operator()(const ast::AssignmentStatement& as) const
		{
		}

		void operator()(const ast::LabelStatement& ls) const
		{
		}

		void operator()(const ast::GotoStatement& gs) const
		{
		}

		void operator()(const ast::BreakStatement&) const
		{
			// Nothing to do here
		}

		void operator()(const ast::DoStatement& ds) const
		{
		}

		void operator()(const ast::WhileStatement& ws) const
		{
		}

		void operator()(const ast::RepeatStatement& rs) const
		{
		}

		void operator()(const ast::IfThenElseStatement& s) const
		{
		}

		void operator()(const ast::NumericalForStatement& s) const
		{
		}

		void operator()(const ast::GenericForStatement& s) const
		{
		}

		void operator()(const ast::FunctionDeclarationStatement& s) const
		{
		}

		void operator()(const ast::LocalFunctionDeclarationStatement& s) const
		{
			auto funcType = getType(m_scope, s.body);
			m_scope.addFunction(s.name, std::move(funcType));
		}

		void operator()(const ast::LocalAssignmentStatement& s) const
		{
			if (!s.expressions)
			{
				for (const auto& v : s.variables)
					m_scope.addVariable(v, {Type::unknown});
			}
			else
			{
				size_t nbV = s.variables.size(), nbE = s.expressions->size();
				const auto& expressions = *s.expressions;
				for (size_t i = 0; i < nbV; ++i)
				{
					TypeInfo type;
					// TODO: support expressions with multiple returns
					if (i < nbE)
						type = getType(m_scope, expressions[i]);
					m_scope.addVariable(s.variables[i], type);
				}
			}
		}

		void operator()(const ast::Block& b) const
		{
			for (const auto& s : b.statements)
				boost::apply_visitor(*this, s);
			if (b.returnStatement)
				(*this)(*b.returnStatement);
		}

	private:
		Scope& m_scope;
	};

	void analyseBlock(Scope& scope, const ast::Block& block)
	{
		AnalysisVisitor{scope}(block);
	}

	Scope analyseBlock(const ast::Block& block, Scope* parentScope)
	{
		Scope scope(parentScope);
		analyseBlock(scope, block);
		return scope;
	}
} // namespace lac::an
