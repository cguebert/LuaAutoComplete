#pragma once

#include <lac/analysis/type_info.h>

#include <map>
#include <string_view>

namespace lac::an
{
	class LAC_API UserDefined
	{
	public:
		void addVariable(std::string_view name, TypeInfo type);
		const TypeInfo* getVariable(std::string_view name) const;

		void addFreeFunction(std::string_view name, FunctionInfo func);
		const TypeInfo* getFunction(std::string_view name) const;

		void addType(UserType type);
		const UserType* getType(std::string_view name) const;

	private:
		std::map<std::string, TypeInfo> m_variables;
		std::map<std::string, TypeInfo> m_functions;
		std::map<std::string, UserType> m_types;
	};
} // namespace lac::an
