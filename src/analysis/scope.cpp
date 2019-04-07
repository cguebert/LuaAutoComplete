#include <analysis/scope.h>

#include <algorithm>

namespace lac::an
{
	Scope::Scope(Scope* parent)
		: m_parent(parent)
	{
	}

	void Scope::addVariable(std::string_view name, TypeInfo type)
	{
		m_variables.push_back(VariableInfo{std::string{name}, type});
	}

	TypeInfo Scope::getVariableType(std::string_view name) const
	{
		const auto it = std::find_if(m_variables.begin(), m_variables.end(), [name](const VariableInfo& v) {
			return v.name == name;
		});
		if (it != m_variables.end())
			return it->type;
		return Type::nil;
	}

	void Scope::addChildScope(Scope scope)
	{
		m_childs.push_back(std::move(scope));
	}
} // namespace lac::an
