#include <lac/analysis/type_info.h>
#include <lac/analysis/parse_type.h>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

namespace lac::an
{
	VariableInfo::VariableInfo(const VariableInfo& other)
		: VariableInfo(other.name(), other.type())
	{
	}

	VariableInfo::VariableInfo(std::string_view name)
		: VariableInfo(name, Type::unknown)
	{
	}

	VariableInfo::VariableInfo(std::string_view name, const TypeInfo& type)
		: m_name(name)
		, m_type(std::make_unique<TypeInfo>(type))
	{
	}

	VariableInfo& VariableInfo::operator=(const VariableInfo& other)
	{
		m_name = other.name();
		m_type = std::make_unique<TypeInfo>(other.type());
		return *this;
	}

	const std::string& VariableInfo::name() const
	{
		return m_name;
	}

	const TypeInfo& VariableInfo::type() const
	{
		return *m_type;
	}

	/****************************************************************************/

	FunctionInfo::FunctionInfo() = default;

	FunctionInfo::FunctionInfo(std::vector<VariableInfo> params, std::vector<TypeInfo> results)
		: parameters(std::move(params))
		, results(std::move(results))
	{
	}

	FunctionInfo::FunctionInfo(std::string_view text, GetResultType getResult, GetCompletion getCompletion)
	{
		setFunction(*this, text);

		// These must be after setFunction
		getResultTypeFunc = getResult;
		getCompletionFunc = getCompletion;
	}

	FunctionInfo::FunctionInfo(const char* text, GetResultType getResult, GetCompletion getCompletion)
		: FunctionInfo(std::string_view{text}, std::move(getResult), std::move(getCompletion))
	{
	}

	/****************************************************************************/

	TypeInfo::TypeInfo() = default;

	TypeInfo::TypeInfo(Type type)
		: type(type)
	{
	}

	TypeInfo::TypeInfo(std::string_view text)
	{
		if (!setType(*this, text))
			type = Type::error;
	}

	TypeInfo::TypeInfo(const char* text)
		: TypeInfo(std::string_view{text})
	{
	}

	TypeInfo::TypeInfo(std::string_view text, FunctionInfo::GetResultType getResult, FunctionInfo::GetCompletion getCompletion)
	{
		if (!setType(*this, text))
			type = Type::error;
		else if (type == Type::function)
		{
			function.getResultTypeFunc = getResult;
			function.getCompletionFunc = getCompletion;
		}
	}

	TypeInfo::TypeInfo(const char* text, FunctionInfo::GetResultType getResult, FunctionInfo::GetCompletion getCompletion)
		: TypeInfo(std::string_view{text}, std::move(getResult), std::move(getCompletion))
	{
	}

	TypeInfo TypeInfo::fromTypeName(std::string_view name)
	{
		if (name == "nil")
			return Type::nil;
		else if (name == "boolean")
			return Type::boolean;
		else if (name == "number"
				 || name == "int"
				 || name == "integer"
				 || name == "float"
				 || name == "double")
			return Type::number;
		else if (name == "string")
			return Type::string;
		else if (name == "table")
			return Type::table;

		TypeInfo type{Type::userdata};
		type.name = name;
		return type;
	}

	TypeInfo TypeInfo::createFunction(std::vector<VariableInfo> parameters, std::vector<TypeInfo> results,
									  FunctionInfo::GetResultType getResult, FunctionInfo::GetCompletion getCompletion)
	{
		TypeInfo type{Type::function};
		type.function.parameters = std::move(parameters);
		type.function.results = std::move(results);
		type.function.isMethod = false;
		type.function.getResultTypeFunc = getResult;
		type.function.getCompletionFunc = getCompletion;
		return type;
	}

	TypeInfo TypeInfo::createMethod(std::vector<VariableInfo> parameters, std::vector<TypeInfo> results,
									FunctionInfo::GetResultType getResult, FunctionInfo::GetCompletion getCompletion)
	{
		TypeInfo type{Type::function};
		type.function.parameters = std::move(parameters);
		type.function.results = std::move(results);
		type.function.isMethod = true;
		type.function.getResultTypeFunc = getResult;
		type.function.getCompletionFunc = getCompletion;
		return type;
	}

	TypeInfo TypeInfo::convert(Type destination) const
	{
		if (type == destination)
			return destination;

		if (type == Type::error)
			return Type::error;
		if (type == Type::unknown)
			return destination;

		switch (destination)
		{
		case Type::boolean:
			return Type::boolean;

		case Type::number:
			if (type == Type::number || type == Type::string)
				return Type::number;
			return Type::error;

		case Type::string:
			if (type == Type::string || type == Type::number)
				return Type::string;
			return Type::error;

		default:
			return Type::error;
		}
	}

	TypeInfo::operator bool() const
	{
		switch (type)
		{
		case Type::nil:
		case Type::unknown:
		case Type::error:
			return false;
		}

		return true;
	}

	bool TypeInfo::hasMember(const std::string& name) const
	{
		return members.count(name) != 0;
	}

	TypeInfo TypeInfo::member(const std::string& name) const
	{
		return members.count(name)
				   ? members.at(name)
				   : TypeInfo{};
	}

	bool TypeInfo::isMethod() const
	{
		return function.isMethod;
	}

	std::string TypeInfo::typeName() const
	{
		if (!name.empty())
		{
			return type == Type::array
					   ? name + "[]"
					   : name;
		}

		switch (type)
		{
		case Type::nil:
			return "nil";
		case Type::boolean:
			return "boolean";
		case Type::number:
			return "number";
		case Type::string:
			return "string";
		case Type::table:
			return "table";
		case Type::function:
			return function.isMethod
					   ? "method"
					   : "function";
		case Type::userdata:
			return "userdata";
		case Type::thread:
			return "thread";
		default:
		case Type::unknown:
			return "unknown";
		case Type::error:
			return "error";
		}
	}

	std::string TypeInfo::functionDefinition() const
	{
		if (type != Type::function)
			return "";

		std::string str;
		const auto& results = function.results;
		if (!results.empty())
		{
			str += results.front().typeName();

			for (size_t i = 1; i < results.size(); ++i)
				str += ", " + results[i].typeName();

			str += ' ';
		}

		str += function.isMethod
				   ? "method("
				   : "function(";

		const auto& args = function.parameters;
		if (!args.empty())
		{
			auto addArg = [&str](const VariableInfo& var) {
				str += var.type().typeName() + ' ' + var.name();
			};

			addArg(args.front());

			for (size_t i = 1; i < args.size(); ++i)
			{
				str += ", ";
				addArg(args[i]);
			}
		}

		str += ')';

		return str;
	}


	TypeInfo TypeInfo::fromJson(const nlohmann::json& json)
	{
		if (json.is_string())
			return TypeInfo{ json.get<std::string>() };

		auto info = TypeInfo::fromTypeName(json["type"]);
		info.name = json.value("name", "");
		info.description = json.value("description", "");

		if (json.contains("members"))
		{
			for (const auto& it : json["members"].items())
				info.members[it.key()] = TypeInfo::fromJson(it.value());
		}

		return info;
	}

	TEST_CASE("Text construction")
	{
		CHECK(TypeInfo{"number"}.type == Type::number);
		CHECK(TypeInfo{"int"}.type == Type::number);
		CHECK(TypeInfo{"double"}.type == Type::number);
		CHECK(TypeInfo{"string"}.type == Type::string);
		CHECK(TypeInfo{"boolean"}.type == Type::boolean);

		TypeInfo info = "int[]";
		CHECK(info.type == Type::array);
		CHECK(info.name == "int");
		CHECK(info.typeName() == "int[]");
		CHECK(info.functionDefinition() == "");

		info = "Player[]";
		CHECK(info.type == Type::array);
		CHECK(info.name == "Player");
		CHECK(info.typeName() == "Player[]");
		CHECK(info.functionDefinition() == "");

		info = "function()";
		CHECK(info.type == Type::function);
		CHECK(info.function.parameters.empty());
		CHECK(info.function.results.empty());
		CHECK_FALSE(info.function.isMethod);
		CHECK(info.typeName() == "function");
		CHECK(info.functionDefinition() == "function()");

		info = "method()";
		CHECK(info.type == Type::function);
		CHECK(info.function.parameters.empty());
		CHECK(info.function.results.empty());
		CHECK(info.function.isMethod);
		CHECK(info.typeName() == "method");
		CHECK(info.functionDefinition() == "method()");

		info = "string function()";
		CHECK(info.type == Type::function);
		CHECK(info.function.parameters.empty());
		REQUIRE(info.function.results.size() == 1);
		CHECK(info.function.results[0].type == Type::string);
		CHECK_FALSE(info.function.isMethod);
		CHECK(info.typeName() == "function");
		CHECK(info.functionDefinition() == "string function()");

		info = "string function(number a)";
		CHECK(info.type == Type::function);
		REQUIRE(info.function.parameters.size() == 1);
		CHECK(info.function.parameters[0].type().type == Type::number);
		CHECK(info.function.parameters[0].name() == "a");
		REQUIRE(info.function.results.size() == 1);
		CHECK(info.function.results[0].type == Type::string);
		CHECK_FALSE(info.function.isMethod);
		CHECK(info.typeName() == "function");
		CHECK(info.functionDefinition() == "string function(number a)");

		info = "Player method(number a, string str)";
		CHECK(info.type == Type::function);
		REQUIRE(info.function.parameters.size() == 2);
		CHECK(info.function.parameters[0].type().type == Type::number);
		CHECK(info.function.parameters[0].name() == "a");
		CHECK(info.function.parameters[1].type().type == Type::string);
		CHECK(info.function.parameters[1].name() == "str");
		REQUIRE(info.function.results.size() == 1);
		CHECK(info.function.results[0].type == Type::userdata);
		CHECK(info.function.results[0].name == "Player");
		CHECK(info.function.isMethod);
		CHECK(info.typeName() == "method");
		CHECK(info.functionDefinition() == "Player method(number a, string str)");
	}

	TEST_CASE("From json")
	{
		auto toJson = [](const std::string& str) {
			return nlohmann::json::parse(str);
		};

		auto info = TypeInfo::fromJson(toJson(R"~~( { "type": "number" } )~~"));
		CHECK(info.type == Type::number);

		info = TypeInfo::fromJson(toJson(R"~~(
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

} // namespace lac::an
