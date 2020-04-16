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

	void Scope::addVariable(const std::string& name, TypeInfo type)
	{
		if (m_userDefined && type.type == Type::function)
		{
			auto inputType = m_userDefined->getScriptInput(name);
			if (inputType)
			{
				// This function is called by the application, and the signature is known
				m_variables[name] = *inputType;
				return;
			}
		}
		m_variables[name] = std::move(type);
	}

	TypeInfo Scope::getVariableType(const std::string& name) const
	{
		const auto it = m_variables.find(name);
		if (it != m_variables.end())
			return it->second;
		if (m_userDefined)
		{
			if (auto var = m_userDefined->getVariable(name))
				return *var;
		}
		if (m_parent)
			return m_parent->getVariableType(name);
		return Type::nil;
	}

	TypeInfo& Scope::modifyTable(const std::string& name)
	{
		const auto it = m_variables.find(name);
		if (it != m_variables.end())
			return it->second;

		m_variables[name] = Type::table;
		return m_variables[name];
	}

	void Scope::addLabel(const std::string& name)
	{
		m_labels.insert(name);
	}

	bool Scope::hasLabel(const std::string& name) const
	{
		if (m_labels.count(name))
			return true;
		if (m_parent)
			return m_parent->hasLabel(name);
		return false;
	}

	TypeInfo Scope::getUserType(std::string_view name) const
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

	const UserDefined* Scope::getUserDefined() const
	{
		return m_parent
				   ? m_parent->getUserDefined()
				   : m_userDefined;
	}

	TypeInfo Scope::resolve(const TypeInfo& type) const
	{
		if (type.type == Type::userdata)
			return getUserType(type.name);
		return type;
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
		auto addVariable = [&elements](const std::string& name, const TypeInfo& type, bool local) {
			if (elements.count(name))
				return;

			Element elt;
			elt.local = local;
			elt.name = name;
			elt.elementType = ElementType::variable;
			elt.typeInfo = type;
			elements[name] = std::move(elt);
		};

		auto addScope = [&](const Scope& scope, bool local) {
			for (const auto& it : scope.m_variables)
				addVariable(it.first, it.second, local);

			if (scope.m_userDefined)
			{
				for (const auto& it : scope.m_userDefined->variables())
					addVariable(it.first, it.second, false);
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
		for (const auto& it : type.members)
		{
			Element elt;
			elt.name = it.first;
			elt.typeInfo = it.second;
			if (it.second.isMethod())
				elt.elementType = ElementType::method;
			elements[it.first] = std::move(elt);
		}

		return elements;
	}

	ElementsMap getElements(const TypeInfo& type, ElementType filter)
	{
		ElementsMap elements;
		for (const auto& it : type.members)
		{
			if (it.second.isMethod() != (filter == ElementType::method))
				continue;

			Element elt;
			elt.name = it.first;
			elt.typeInfo = it.second;
			if (it.second.isMethod())
				elt.elementType = ElementType::method;
			elements[it.first] = std::move(elt);
		}

		return elements;
	}

} // namespace lac::an
