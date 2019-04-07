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
		m_variables.push_back(VariableInfo{std::string{name}, std::move(type)});
	}

	TypeInfo Scope::getVariableType(std::string_view name) const
	{
		const auto it = std::find_if(m_variables.begin(), m_variables.end(), [name](const VariableInfo& v) {
			return v.name == name;
		});
		if (it != m_variables.end())
			return it->type;
		if (m_parent)
			return m_parent->getVariableType(name);
		return Type::nil;
	}

	void Scope::addLabel(std::string_view name)
	{
		m_labels.push_back(LabelInfo{std::string{name}});
	}

	bool Scope::hasLabel(std::string_view name) const
	{
		const auto it = std::find_if(m_labels.begin(), m_labels.end(), [name](const LabelInfo& l) {
			return l.name == name;
		});
		if (it != m_labels.end())
			return true;
		if (m_parent)
			return m_parent->hasLabel(name);
		return false;
	}

	void Scope::addFunction(std::string_view name, TypeInfo type)
	{
		m_functions.push_back(FunctionInfo{std::string{name}, std::move(type)});
	}

	TypeInfo Scope::getFunctionType(std::string_view name) const
	{
		const auto it = std::find_if(m_functions.begin(), m_functions.end(), [name](const FunctionInfo& f) {
			return f.name == name;
		});
		if (it != m_functions.end())
			return it->type;
		if (m_parent)
			return m_parent->getFunctionType(name);
		return Type::nil;
	}

	Scope& Scope::getGlobalScope()
	{
		return m_parent
				   ? m_parent->getGlobalScope()
				   : *this;
	}

	void Scope::addChildScope(Scope&& scope)
	{
		m_childs.push_back(std::move(scope));
	}
} // namespace lac::an
