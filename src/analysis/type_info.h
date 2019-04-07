#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
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
	struct Parameter
	{
		Parameter(const Parameter& other);
		Parameter(std::string_view name);
		Parameter(std::string_view name, const TypeInfo& type);
		Parameter(Parameter&&) = default;

		const std::string name;
		const std::unique_ptr<TypeInfo> type;
	};

	class TypeInfo
	{
	public:
		TypeInfo();
		TypeInfo(Type type);

		// Returns destination if possible, error otherwise
		TypeInfo convert(Type destination) const;

		// Returns false only if type is error
		operator bool() const;

		Type type = Type::nil;

		// For tables
		std::map<std::string, TypeInfo> members;

		// For functions
		std::vector<Parameter> parameters;
		std::vector<TypeInfo> results;
	};
} // namespace lac::an
