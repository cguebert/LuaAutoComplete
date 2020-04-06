#pragma once

#include <lac/analysis/type_info.h>

#include <map>
#include <string_view>

namespace lac::an
{
	class LAC_API UserDefined
	{
	public:
		using Variables = std::map<std::string, TypeInfo>;
		using Functions = std::map<std::string, TypeInfo>;
		using Types = std::map<std::string, TypeInfo>;

		void addVariable(std::string_view name, TypeInfo type);
		const TypeInfo* getVariable(std::string_view name) const;
		const Variables& variables() const;

		void addFreeFunction(std::string_view name, FunctionInfo func);
		const TypeInfo* getFunction(std::string_view name) const;
		const Functions& functions() const;

		void addScriptInput(std::string_view name, FunctionInfo func); // Setup the signature of a function called by the application
		const TypeInfo* getScriptInput(std::string_view name) const;
		const Functions& scriptInputs() const;

		void addType(TypeInfo type);
		const TypeInfo* getType(std::string_view name) const;
		const Types& types() const;

	private:
		Variables m_variables;
		Functions m_functions;
		Functions m_scriptEntries;
		Types m_types;
	};
} // namespace lac::an
