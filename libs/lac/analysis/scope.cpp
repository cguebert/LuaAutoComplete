#include <lac/analysis/scope.h>
#include <lac/analysis/user_defined.h>

#include <algorithm>

namespace lac::an
{
	Scope::Scope(const ast::Block& block, Scope* parent)
		: m_block(&block)
		, m_parent(parent)
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
		if (m_userDefined)
		{
			if (auto var = m_userDefined->getVariable(name))
				return *var;
		}
		if (m_parent)
			return m_parent->getVariableType(name);
		return Type::nil;
	}

	TypeInfo& Scope::modifyTable(std::string_view name)
	{
		const auto it = std::find_if(m_variables.begin(), m_variables.end(), [name](const VariableInfo& v) {
			return v.name == name;
		});
		if (it != m_variables.end())
			return it->type;

		m_variables.push_back(VariableInfo{std::string{name}, Type::table});
		return m_variables.back().type;
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
		if (m_userDefined)
		{
			if (auto func = m_userDefined->getFunction(name))
				return *func;
		}
		if (m_parent)
			return m_parent->getFunctionType(name);
		return Type::nil;
	}

	UserType Scope::getUserType(std::string_view name) const
	{
		if (m_userDefined)
		{
			if (auto user = m_userDefined->getType(name))
				return *user;
		}

		if (m_parent)
			return m_parent->getUserType(name);
		return {};
	}

	Scope& Scope::getGlobalScope()
	{
		return m_parent
				   ? m_parent->getGlobalScope()
				   : *this;
	}

	void Scope::addChildScope(Scope&& scope)
	{
		m_children.push_back(std::move(scope));
	}

	void Scope::setUserDefined(UserDefined* userDefined)
	{
		getGlobalScope().m_userDefined = userDefined;
	}

	UserDefined* Scope::getUserDefined() const
	{
		return m_parent
				   ? m_parent->getUserDefined()
				   : m_userDefined;
	}

	const ast::Block* Scope::block() const
	{
		return m_block;
	}

	const std::vector<Scope>& Scope::children() const
	{
		// This is a sort of a hack, but we have to ensure that the parent pointer is valid
		// as it can have been invalidated by other scopes being added to a vector
		auto mutThis = const_cast<Scope*>(this);
		for (auto& child : mutThis->m_children)
			child.m_parent = mutThis;

		return m_children;
	}

	std::map<std::string, Element> Scope::getElements(bool localOnly) const
	{
		std::map<std::string, Element> elements;
		auto addScope = [&elements](const Scope& scope, bool local) {
			for (const auto& var : scope.m_variables)
			{
				if (elements.count(var.name))
					continue;

				Element elt;
				elt.local = local;
				elt.name = var.name;
				elt.elementType = ElementType::variable;
				elt.typeInfo = var.type;
				elements[var.name] = std::move(elt);
			}

			for (const auto& func : scope.m_functions)
			{
				if (elements.count(func.name))
					continue;

				Element elt;
				elt.local = local;
				elt.name = func.name;
				elt.elementType = ElementType::label;
				elt.typeInfo = func.type;
				elements[func.name] = std::move(elt);
			}

			for (const auto& label : scope.m_labels)
			{
				if (elements.count(label.name))
					continue;

				Element elt;
				elt.local = local;
				elt.name = label.name;
				elt.elementType = ElementType::label;
				elements[label.name] = std::move(elt);
			}
		};

		addScope(*this, true);
		if (!localOnly)
		{
			auto parent = m_parent;
			while (parent)
			{
				addScope(*parent, false);
				parent = parent->m_parent;
			}
		}
		return elements;
	}

	ElementsMap getElements(const TypeInfo& type)
	{
		ElementsMap elements;
		for (const auto& member : type.members)
		{
			Element elt;
			elt.name = member.first;
			elt.typeInfo = member.second;
			if (member.second.type == Type::function)
				elt.elementType = ElementType::function;
			elements[member.first] = std::move(elt);
		}

		return elements;
	}

} // namespace lac::an
