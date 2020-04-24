#include <lac/analysis/analyze_block.h>
#include <lac/analysis/user_defined.h>
#include <lac/completion/completion.h>
#include <lac/completion/function_at_pos.h>
#include <lac/completion/get_block.h>
#include <lac/completion/type_at_pos.h>
#include <lac/completion/variable_at_pos.h>
#include <lac/parser/parser.h>

#include <doctest/doctest.h>

namespace lac::comp
{
	boost::optional<ArgumentData> getArgumentAtPos(const an::Scope& rootScope, std::string_view view, size_t pos)
	{
		if (pos == std::string_view::npos)
			pos = view.size() - 1;

		if (pos >= view.size())
			return {};

		auto ps = pos;
		size_t index = 0; // Argument index

		// Go left while the character is a whitespace
		auto ignoreWhiteSpace = [&ps, &view] {
			while (ps != 0 && std::isspace(view[ps - 1]))
				--ps;
		};

		// Go left until we encounter the opening parenthesis
		auto goToCallStart = [&ps, &view, &index] {
			int nbParen = 1;
			while (ps != 0)
			{
				--ps;

				// TODO: test if these characters are inside a litteral string or a comment
				if (view[ps] == ')')
					++nbParen;
				else if (view[ps] == '(')
				{
					--nbParen;
					if (!nbParen)
						break;
				}
				else if (view[ps] == ',')
					++index;
			}
		};

		if (view[ps] != '(')
		{
			goToCallStart();
			if (!ps)
				return {};
		}
		--ps;

		ignoreWhiteSpace();

		// Parse what is under the cursor
		const auto var = parseVariableAtPos(view, ps);
		if (!var)
			return {};

		// Get the scope under the cursor
		auto scope = pos::getScopeAtPos(rootScope, ps);
		if (!scope)
			return {};

		// Get the function type
		const auto functionType = getVariableType(*scope, *var);
		if (functionType.type != an::Type::function)
			return {};

		// Get the type of the variable before the function call (the object for a method or the table for a function)
		const auto parentVar = removeLastPart(*var);
		const an::TypeInfo parentType = parentVar
											? getVariableType(*scope, *parentVar)
											: an::Type::nil;

		return ArgumentData{parentType, functionType, index};
	}

	TEST_CASE("Simple functions")
	{
		an::Scope parentScope;
		parentScope.addVariable("negate", "number function(number a)");
		parentScope.addVariable("mult", "number function(number a, number b)");

		const std::string program = R"~~(
x = negate(42)
y = mult(x, 10)
)~~";

		// Parse the program
		const auto ret = parser::parseBlock(program);
		REQUIRE(ret.parsed);

		// Analyse the program
		auto scope = an::analyseBlock(ret.block, &parentScope);

		auto data = getArgumentAtPos(scope, program, 11); // negate(
		REQUIRE(data.has_value());
		CHECK(data->parent.type == an::Type::nil);
		CHECK(data->function.type == an::Type::function);
		CHECK(data->function.function.parameters.size() == 1);
		CHECK(data->argumentIndex == 0);

		data = getArgumentAtPos(scope, program, 12); // (42
		REQUIRE(data.has_value());
		CHECK(data->parent.type == an::Type::nil);
		CHECK(data->function.type == an::Type::function);
		CHECK(data->function.function.parameters.size() == 1);
		CHECK(data->argumentIndex == 0);

		data = getArgumentAtPos(scope, program, 25); // (x
		REQUIRE(data.has_value());
		CHECK(data->parent.type == an::Type::nil);
		CHECK(data->function.type == an::Type::function);
		CHECK(data->function.function.parameters.size() == 2);
		CHECK(data->argumentIndex == 0);

		data = getArgumentAtPos(scope, program, 28); // (x, 10
		REQUIRE(data.has_value());
		CHECK(data->parent.type == an::Type::nil);
		CHECK(data->function.type == an::Type::function);
		CHECK(data->function.function.parameters.size() == 2);
		CHECK(data->argumentIndex == 1);
	}

	TEST_CASE("Simple methods")
	{
		an::UserDefined userDefined;
		an::TypeInfo vec3Type = an::Type::table;
		vec3Type.name = "Vector3";
		vec3Type.members["x"] = an::Type::number;
		vec3Type.members["y"] = an::Type::number;
		vec3Type.members["z"] = an::Type::number;
		vec3Type.members["length"] = "number method()";
		vec3Type.members["mult"] = "Vector3 method(number v)";
		userDefined.addType(std::move(vec3Type));

		an::TypeInfo playerType = an::Type::table;
		playerType.name = "Player";
		playerType.members["name"] = an::Type::string;
		playerType.members["id"] = "number method()";
		playerType.members["position"] = "Vector3 method()";
		playerType.members["setPosition"] = "method(Vector3 position)";
		userDefined.addType(std::move(playerType));

		userDefined.addVariable("player", "Player");

		an::Scope parentScope;
		parentScope.setUserDefined(&userDefined);

		const std::string program = R"~~(
pos = player:position()
len = pos:length()
pos = pos:mult(1.1)
)~~";

		// Parse the program
		const auto ret = parser::parseBlock(program);
		REQUIRE(ret.parsed);

		// Analyse the program
		auto scope = an::analyseBlock(ret.block, &parentScope);

		auto data = getArgumentAtPos(scope, program, 22);
		REQUIRE(data.has_value());
		CHECK(data->parent.type == an::Type::table);
		CHECK(data->parent.name == "Player");
		CHECK(data->function.type == an::Type::function);
		CHECK(data->function.function.parameters.size() == 0);
		CHECK(data->argumentIndex == 0);

		data = getArgumentAtPos(scope, program, 41);
		REQUIRE(data.has_value());
		CHECK(data->parent.name == "Vector3");
		CHECK(data->parent.type == an::Type::table);
		CHECK(data->function.type == an::Type::function);
		CHECK(data->function.function.parameters.size() == 0);
		CHECK(data->argumentIndex == 0);

		data = getArgumentAtPos(scope, program, 58);
		REQUIRE(data.has_value());
		CHECK(data->parent.name == "Vector3");
		CHECK(data->parent.type == an::Type::table);
		CHECK(data->function.type == an::Type::function);
		CHECK(data->function.function.parameters.size() == 1);
		CHECK(data->argumentIndex == 0);
	}
} // namespace lac::comp
