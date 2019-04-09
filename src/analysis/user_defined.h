#pragma once

#include <analysis/type_info.h>

#include <map>
#include <string>

namespace lac::an
{
	class UserDefined
	{
	public:
		void addVariable(const std::string& name, TypeInfo type);
		const TypeInfo* getVariable(const std::string& name) const;

		void addFreeFunction(const std::string& name, FunctionInfo func);
		const FunctionInfo* getFunction(const std::string& name) const;

		void addType(UserType type);
		const UserType* getType(const std::string& name) const;

	private:
		std::map<std::string, TypeInfo> m_variables;
		std::map<std::string, FunctionInfo> m_functions;
		std::map<std::string, UserType> m_types;
	};
} // namespace lac::an
