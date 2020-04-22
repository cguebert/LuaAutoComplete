#include <lac/analysis/get_sub_type.h>
#include <lac/analysis/get_type.h>
#include <lac/analysis/scope.h>
#include <lac/parser/ast.h>

namespace lac::an
{
	class GetSubType : public boost::static_visitor<TypeInfo>
	{
	public:
		GetSubType(const Scope& scope, const TypeInfo& parentType)
			: m_scope(scope)
			, m_parentType(parentType)
		{
		}

		TypeInfo operator()(const ast::PostPrefix& pp) const
		{
			return boost::apply_visitor(*this, pp);
		}

		TypeInfo operator()(const ast::TableIndexExpression& tie) const
		{
			const auto parent = parentAsVariable();
			if (parent.type == Type::array && getType(m_scope, tie.expression).type == Type::number)
				return TypeInfo::fromTypeName(parent.name);

			return {};
		}

		TypeInfo operator()(const ast::TableIndexName& tin) const
		{
			return parentAsVariable().member(tin.name);
		}

		TypeInfo operator()(const ast::FunctionCallEnd& fce) const
		{
			const auto parent = parentAsVariable();
			const auto type = fce.member
								  ? parent.member(*fce.member)
								  : parent;

			if (type.function.getResultTypeFunc)
				return type.function.getResultTypeFunc(m_scope, fce.arguments, parent);

			if (type.function.results.empty())
				return {};
			return type.function.results.front(); // TODO: return all results
		}

		TypeInfo operator()(const ast::VariablePostfix& vp) const
		{
			return boost::apply_visitor(*this, vp);
		}

		TypeInfo operator()(const ast::VariableFunctionCall& vfc) const
		{
			const auto funcType = getSubType(m_scope, m_parentType, vfc.functionCall);
			return getSubType(m_scope, funcType, vfc.postVariable);
		}

	private:
		TypeInfo parentAsVariable() const
		{
			if (m_parentType.type == Type::unknown && !m_parentType.name.empty())
			{
				auto info = m_scope.getVariableType(m_parentType.name);
				if (!info)
					info = m_scope.getUserType(m_parentType.name);
				return m_scope.resolve(info);
			}
			return m_scope.resolve(m_parentType);
		}

		const Scope& m_scope;
		const TypeInfo& m_parentType;
	};

	TypeInfo getSubType(const Scope& scope, const TypeInfo& parentType, const ast::PostPrefix& pp)
	{
		return GetSubType{scope, parentType}(pp);
	}

	TypeInfo getSubType(const Scope& scope, const TypeInfo& parentType, const ast::VariablePostfix& vp)
	{
		return GetSubType{scope, parentType}(vp);
	}

	TypeInfo getSubType(const Scope& scope, const TypeInfo& parentType, const ast::FunctionCallEnd& fce)
	{
		return GetSubType{scope, parentType}(fce);
	}

} // namespace lac::an
