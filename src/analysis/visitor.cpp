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
			// Nothing to do here
		}

		void operator()(int v) const
		{
			// Nothing to do here
		}

		void operator()(double v) const
		{
			// Nothing to do here
		}

		void operator()(const std::string& str) const
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

		void operator()(const ast::FunctionBody& fb) const
		{
			Scope scope{&m_scope};
			if (fb.parameters)
			{
				for (const auto& p : fb.parameters->parameters)
					scope.addVariable(p, Type::unknown);
			}

			analyseBlock(scope, fb.block);
			m_scope.addChildScope(std::move(scope));
		}

		void operator()(const ast::Operand& op) const
		{
			boost::apply_visitor(*this, op);
		}

		void operator()(const ast::Expression& ex) const
		{
			(*this)(ex.operand);
			if (ex.binaryOperation)
				(*this)(*ex.binaryOperation);
		}

		void operator()(const ast::ExpressionsList& el) const
		{
			for (const auto& ex : el)
				(*this)(ex);
		}

		void operator()(const ast::BracketedExpression be) const
		{
			(*this)(be.expression);
		}

		void operator()(const ast::TableIndexExpression tie) const
		{
			(*this)(tie.expression);
		}

		void operator()(const ast::TableIndexName& tin) const
		{
			// Nothing to do here
		}

		void operator()(const ast::EmptyArguments& arg) const
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

		void operator()(const ast::FunctionCall& fc) const
		{
			(*this)(fc.functionCall);
		}

		void operator()(const ast::ReturnStatement& rs) const
		{
			for (const auto& ex : rs.expressions)
				(*this)(ex);
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
			m_scope.addLabel(ls.name);
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
			m_scope.addChildScope(analyseBlock(ds.block, &m_scope));
		}

		void operator()(const ast::WhileStatement& ws) const
		{
			m_scope.addChildScope(analyseBlock(ws.block, &m_scope));
		}

		void operator()(const ast::RepeatStatement& rs) const
		{
			m_scope.addChildScope(analyseBlock(rs.block, &m_scope));
		}

		void operator()(const ast::IfThenElseStatement& s) const
		{
			m_scope.addChildScope(analyseBlock(s.first.block, &m_scope));
			for (const auto& es : s.rest)
				m_scope.addChildScope(analyseBlock(es.block, &m_scope));
			if (s.elseBlock)
				m_scope.addChildScope(analyseBlock(*s.elseBlock, &m_scope));
		}

		void operator()(const ast::NumericalForStatement& s) const
		{
			Scope scope{&m_scope};
			scope.addVariable(s.variable, Type::number);
			analyseBlock(scope, s.block);
			m_scope.addChildScope(std::move(scope));
		}

		void operator()(const ast::GenericForStatement& s) const
		{
			Scope scope{&m_scope};
			size_t nbV = s.variables.size(), nbE = s.expressions.size();
			for (size_t i = 0; i < nbV; ++i)
			{
				TypeInfo type;
				// TODO: support expressions with multiple returns
				if (i < nbE)
					type = getType(m_scope, s.expressions[i]);
				m_scope.addVariable(s.variables[i], type);
			}

			analyseBlock(scope, s.block);
			m_scope.addChildScope(std::move(scope));
		}

		void operator()(const ast::FunctionDeclarationStatement& s) const
		{
			// TODO: it can be global function declaration
			// or the declaration of a table method
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

				// Visit expressions, add child scopes
				for (const auto& e : *s.expressions)
					(*this)(e);
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
