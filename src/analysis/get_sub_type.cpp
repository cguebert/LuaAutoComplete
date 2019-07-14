#include <analysis/get_sub_type.h>
#include <analysis/scope.h>

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
			return {};
		}

		TypeInfo operator()(const ast::TableIndexName& tin) const
		{
			const auto parent = parentAsVariable();
			if (parent.type == Type::table)
			{
				return parent.members.count(tin.name)
						   ? parent.members.at(tin.name)
						   : TypeInfo{};
			}
			else if (parent.type == Type::userdata)
			{
				const auto userType = m_scope.getUserType(parent.userType);
				if (userType.name.empty())
					return {};

				return userType.getVariableType(tin.name);
			}

			return {};
		}

		TypeInfo operator()(const ast::FunctionCallEnd& fce) const
		{
			const auto parent = parentAsFunction();
			if (parent.function.results.empty())
				return {};
			return parent.function.results.front(); // TODO: return all results
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
			if (m_parentType.type == Type::string)
				return m_scope.getVariableType(m_parentType.name);
			return m_parentType;
		}

		TypeInfo parentAsFunction() const
		{
			if (m_parentType.type == Type::string)
				return m_scope.getFunctionType(m_parentType.name);
			return m_parentType;
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
