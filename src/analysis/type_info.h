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
		thread
	};

	class TypeInfo
	{
	public:
		TypeInfo();
		TypeInfo(Type type);

		Type type = Type::nil;

		// For tables
		std::map<std::string, TypeInfo> members;

		// For functions
		std::vector<TypeInfo> parameters;
		std::vector<TypeInfo> results;
	};
} // namespace lac::an
