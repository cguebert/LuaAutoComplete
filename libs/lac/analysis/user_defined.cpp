#include <lac/analysis/user_defined.h>

#include <nlohmann/json.hpp>

namespace lac::an
{
	void UserDefined::addVariable(std::string_view name, TypeInfo type)
	{
		variables[std::string{name}] = type;
	}

	const TypeInfo* UserDefined::getVariable(std::string_view name) const
	{
		const auto it = variables.find(std::string{name});
		if (it != variables.end())
			return &it->second;
		return nullptr;
	}

	void UserDefined::addScriptInput(std::string_view name, TypeInfo info)
	{
		if(info.type == Type::function)
			scriptEntries[std::string{name}] = std::move(info);
	}

	const TypeInfo* UserDefined::getScriptInput(std::string_view name) const
	{
		const auto it = scriptEntries.find(std::string{name});
		if (it != scriptEntries.end())
			return &it->second;
		return nullptr;
	}

	void UserDefined::addType(TypeInfo type)
	{
		auto name = type.name;
		if (name.empty())
			name = "no_name#" + std::to_string(types.size());
		types[type.name] = std::move(type);
	}

	const TypeInfo* UserDefined::getType(std::string_view name) const
	{
		const auto it = types.find(std::string{name});
		if (it != types.end())
			return &it->second;
		return nullptr;
	}

	void UserDefined::addFromJson(const std::string& str)
	{
		const auto json = nlohmann::json::parse(str);

		if (json.contains("types"))
		{
			for (const auto& type : json["types"])
				addType(TypeInfo::fromJson(type));
		}

		if (json.contains("variables"))
		{
			for (const auto& it : json["variables"].items())
				addVariable(it.key(), TypeInfo::fromJson(it.value()));
		}

		if (json.contains("script_inputs"))
		{
			for (const auto& it : json["script_inputs"].items())
				addScriptInput(it.key(), TypeInfo::fromJson(it.value()));
		}
	}

} // namespace lac::an
