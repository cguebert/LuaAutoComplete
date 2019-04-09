#include <analysis/user_defined.h>

namespace lac::an
{
	void UserDefined::addVariable(const std::string& name, TypeInfo type)
	{
		m_variables[name] = type;
	}
	
	const TypeInfo* UserDefined::getVariable(const std::string& name) const
	{
		const auto it = m_variables.find(name);
		if (it != m_variables.end())
			return &it->second;
		return nullptr;
	}

	void UserDefined::addFreeFunction(const std::string& name, FunctionInfo func)
	{
		m_functions[name] = std::move(func);
	}

	const FunctionInfo* UserDefined::getFunction(const std::string& name) const
	{
		const auto it = m_functions.find(name);
		if (it != m_functions.end())
			return &it->second;
		return nullptr;
	}

	void UserDefined::addType(UserType type)
	{
		m_types[type.name] = std::move(type);
	}

	const UserType* UserDefined::getType(const std::string& name) const
	{
		const auto it = m_types.find(name);
		if (it != m_types.end())
			return &it->second;
		return nullptr;
	}
} // namespace lac::an
