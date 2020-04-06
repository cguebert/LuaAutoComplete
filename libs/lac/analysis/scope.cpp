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
		if (m_userDefined)
		{
			auto inputType = m_userDefined->getScriptInput(name);
			if (inputType)
			{
				// This function is called by the application, and the signature is known
				m_functions.push_back(FunctionInfo{std::string{name}, *inputType});
				return;
			}
		}
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

	const UserDefined* Scope::getUserDefined() const
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

		auto addFunction = [&elements](const std::string& name, const TypeInfo& type, bool local) {
			if (elements.count(name))
				return;

			Element elt;
			elt.local = local;
			elt.name = name;
			elt.elementType = ElementType::label;
			elt.typeInfo = type;
			elements[name] = std::move(elt);
		};

		auto addScope = [&](const Scope& scope, bool local) {
			for (const auto& var : scope.m_variables)
				addVariable(var.name, var.type, local);

			for (const auto& func : scope.m_functions)
				addFunction(func.name, func.type, local);

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

			if (scope.m_userDefined)
			{
				for (const auto& it : scope.m_userDefined->variables())
					addVariable(it.first, it.second, false);

				for (const auto& it : scope.m_userDefined->functions())
					addFunction(it.first, it.second, false);
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
			if (it.second.type == Type::function)
				elt.elementType = ElementType::function;
			elements[it.first] = std::move(elt);
		}

		return elements;
	}

	ElementsMap getElements(const TypeInfo& type, ElementType filter)
	{
		ElementsMap elements;
		for (const auto& it : type.members)
		{
			if (it.second.type == Type::function && filter == ElementType::variable)
				continue;
			if (it.second.type != Type::function && filter == ElementType::function)
				continue;

			Element elt;
			elt.name = it.first;
			elt.typeInfo = it.second;
			if (it.second.type == Type::function)
				elt.elementType = ElementType::function;
			elements[it.first] = std::move(elt);
		}

		return elements;
	}

} // namespace lac::an
