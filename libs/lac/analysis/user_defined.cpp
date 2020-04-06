#include <lac/analysis/user_defined.h>

namespace lac::an
{
	void UserDefined::addVariable(std::string_view name, TypeInfo type)
	{
		m_variables[std::string{name}] = type;
	}

	const TypeInfo* UserDefined::getVariable(std::string_view name) const
	{
		const auto it = m_variables.find(std::string{name});
		if (it != m_variables.end())
			return &it->second;
		return nullptr;
	}

	const UserDefined::Variables& UserDefined::variables() const
	{
		return m_variables;
	}

	void UserDefined::addFreeFunction(std::string_view name, FunctionInfo func)
	{
		TypeInfo info = Type::function;
		info.function = std::move(func);
		m_functions[std::string{name}] = std::move(info);
	}

	const TypeInfo* UserDefined::getFunction(std::string_view name) const
	{
		const auto it = m_functions.find(std::string{name});
		if (it != m_functions.end())
			return &it->second;
		return nullptr;
	}

	const UserDefined::Functions& UserDefined::functions() const
	{
		return m_functions;
	}

	void UserDefined::addScriptInput(std::string_view name, FunctionInfo func)
	{
		TypeInfo info = Type::function;
		info.function = std::move(func);
		m_scriptEntries[std::string{name}] = std::move(info);
	}
	
	const TypeInfo* UserDefined::getScriptInput(std::string_view name) const
	{
		const auto it = m_scriptEntries.find(std::string{name});
		if (it != m_scriptEntries.end())
			return &it->second;
		return nullptr;
	}
	
	const UserDefined::Functions& UserDefined::scriptInputs() const
	{
		return m_scriptEntries;
	}

	void UserDefined::addType(TypeInfo type)
	{
		m_types[type.name] = std::move(type);
	}

	const TypeInfo* UserDefined::getType(std::string_view name) const
	{
		const auto it = m_types.find(std::string{name});
		if (it != m_types.end())
			return &it->second;
		return nullptr;
	}

	const UserDefined::Types& UserDefined::types() const
	{
		return m_types;
	}
} // namespace lac::an
