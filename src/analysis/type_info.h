#pragma once

#include <map>
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
		std::vector<TypeInfo> parameters;
		std::vector<TypeInfo> results;
	};
} // namespace lac::an
