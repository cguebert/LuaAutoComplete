#include <lac/analysis/user_defined.h>

#include <doctest/doctest.h>
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
		if (info.type == Type::function)
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

#ifdef WITH_NLOHMANN_JSON
	TypeInfo typeFromJson(const nlohmann::json& json)
	{
		if (json.is_string())
			return TypeInfo{json.get<std::string>()};

		auto info = TypeInfo::fromTypeName(json["type"]);
		info.name = json.value("name", "");
		info.description = json.value("description", "");

		if (json.contains("members"))
		{
			for (const auto& it : json["members"].items())
				info.members[it.key()] = typeFromJson(it.value());
		}

		return info;
	}

	void UserDefined::addFromJson(const std::string& str)
	{
		const auto json = nlohmann::json::parse(str);

		if (json.contains("types"))
		{
			for (const auto& type : json["types"])
				addType(typeFromJson(type));
		}

		if (json.contains("variables"))
		{
			for (const auto& it : json["variables"].items())
				addVariable(it.key(), typeFromJson(it.value()));
		}

		if (json.contains("script_inputs"))
		{
			for (const auto& it : json["script_inputs"].items())
				addScriptInput(it.key(), typeFromJson(it.value()));
		}
	}

	TEST_CASE("From json")
	{
		auto toJson = [](const std::string& str) {
			return nlohmann::json::parse(str);
		};

		auto info = typeFromJson(toJson(R"~~( { "type": "number" } )~~"));
		CHECK(info.type == Type::number);

		info = typeFromJson(toJson(R"~~(
{
	"type": "table",
	"name": "Player",
	"description": "Each player in the game",
	"members": {
		"id": "number",
		"position": "Pos method()"
	}
}
)~~"));
		CHECK(info.type == Type::table);
		CHECK(info.name == "Player");
		CHECK(info.description == "Each player in the game");
		REQUIRE(info.members.size() == 2);
		REQUIRE(info.members.count("id"));
		REQUIRE(info.members["id"].type == Type::number);
		REQUIRE(info.members.count("position"));
		REQUIRE(info.members["position"].type == Type::function);
		REQUIRE(info.members["position"].functionDefinition() == "Pos method()");
	}
#endif

} // namespace lac::an
