#pragma once

#include <lac/core_api.h>

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace lac::ast
{
	struct Arguments;
}

namespace lac::an
{
	enum class Type
	{
		nil,
		boolean,
		number,
		string,
		table,
		array,
		function,
		userdata,
		thread,

		unknown,
		error
	};

	class Scope;
	class TypeInfo;
	class CORE_API VariableInfo
	{
	public:
		VariableInfo(const VariableInfo& other);
		VariableInfo(std::string_view name);
		VariableInfo(std::string_view name, const TypeInfo& type);
		VariableInfo(VariableInfo&&) = default;
		VariableInfo& operator=(const VariableInfo& other);
		VariableInfo& operator=(VariableInfo&&) = default;

		const std::string& name() const;
		const TypeInfo& type() const;

	private:
		std::string m_name;
		std::unique_ptr<TypeInfo> m_type; // Unique_ptr to break the circular dependency
	};

	struct FunctionInfo
	{
		using GetResultType = std::function<TypeInfo(const Scope& scope, const ast::Arguments&)>;

		std::vector<VariableInfo> parameters;
		std::vector<TypeInfo> results;
		bool isMethod = false;
		GetResultType getResultTypeFunc;
	};

	class CORE_API TypeInfo
	{
	public:
		TypeInfo();
		TypeInfo(Type type);

		static TypeInfo fromTypeName(std::string_view name); // For used-defined types
		static TypeInfo fromName(std::string_view name); // For either variable or function names (when we do not yet know) 
		static TypeInfo createFunction(std::vector<VariableInfo> parameters, std::vector<TypeInfo> results = {}, FunctionInfo::GetResultType func = {});
		static TypeInfo createMethod(std::vector<VariableInfo> parameters, std::vector<TypeInfo> results = {}, FunctionInfo::GetResultType func = {});

		// Returns destination if possible, error otherwise
		TypeInfo convert(Type destination) const;

		// Returns false if nil, unknown or error
		operator bool() const;

		Type type = Type::nil;

		// For tables
		std::map<std::string, TypeInfo> members;
		bool hasMember(const std::string& name) const;
		TypeInfo member(const std::string& name) const;

		// For functions
		FunctionInfo function;
		bool isMethod() const;

		// For variable name, function name, or user-defined type
		std::string name;

		std::string typeName() const; // Return the type name, and convert basic types to string

		std::any custom; // For any custom data we may need
	};
} // namespace lac::an
