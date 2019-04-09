#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace lac::an
{
	enum Type
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
		std::vector<VariableInfo> variables;
		std::vector<FunctionInfo> methods;
	};

	class TypeInfo
	{
	public:
		TypeInfo();
		TypeInfo(Type type);
		TypeInfo(std::string_view name); // User-defined type name

		// Returns destination if possible, error otherwise
		TypeInfo convert(Type destination) const;

		// Returns false only if type is error
		operator bool() const;

		Type type = Type::nil;

		// For tables
		std::map<std::string, TypeInfo> members;

		// For functions
		FunctionInfo function;

		// For userdata
		std::string userType;
	};
} // namespace lac::an
