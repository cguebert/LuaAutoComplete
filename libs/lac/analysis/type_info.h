#pragma once

#include <lac/lac_api.h>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace lac::an
{
	enum class Type
	{
		nil,
		boolean,
		number,
		string,
		table,
		function,
		userdata,
		thread,

		unknown,
		error
	};

	class TypeInfo;
	class VariableInfo
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
		std::vector<VariableInfo> parameters;
		std::vector<TypeInfo> results;
	};

	struct UserType
	{
		std::string name;
		std::map<std::string, TypeInfo> variables;
		std::map<std::string, FunctionInfo> methods;

		TypeInfo getVariableType(const std::string& name) const;
		FunctionInfo getMethodType(const std::string& name) const;
	};

	class LAC_API TypeInfo
	{
	public:
		TypeInfo();
		TypeInfo(Type type);

		static TypeInfo fromTypeName(std::string_view name); // For used-defined types
		static TypeInfo fromName(std::string_view name); // For either variable or function names (when we do not yet know) 

		// Returns destination if possible, error otherwise
		TypeInfo convert(Type destination) const;

		// Returns false only if type is error
		operator bool() const;

		Type type = Type::nil;

		// For tables
		std::map<std::string, TypeInfo> members;
		TypeInfo member(const std::string& name) const;

		// For functions
		FunctionInfo function;

		// For variable name, function name, or user-defined type
		std::string name;

		std::string typeName() const; // Return the type name, and convert basic types to string
	};
} // namespace lac::an
