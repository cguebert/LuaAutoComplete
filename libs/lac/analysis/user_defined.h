#pragma once

#include <lac/analysis/type_info.h>

#include <map>
#include <string_view>

namespace lac::an
{
	class CORE_API UserDefined
	{
	public:
		using TypeMap = std::map<std::string, TypeInfo>;

		void addVariable(std::string_view name, TypeInfo type);
		const TypeInfo* getVariable(std::string_view name) const;
		const TypeMap& variables() const;

		void addScriptInput(std::string_view name, FunctionInfo func); // Setup the signature of a function called by the application
		const TypeInfo* getScriptInput(std::string_view name) const;
		const TypeMap& scriptInputs() const;

		void addType(TypeInfo type);
		const TypeInfo* getType(std::string_view name) const;
		const TypeMap& types() const;

	private:
		TypeMap m_variables;
		TypeMap m_scriptEntries;
		TypeMap m_types;
	};
} // namespace lac::an
