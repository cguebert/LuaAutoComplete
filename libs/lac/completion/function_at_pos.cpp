#include <lac/analysis/analyze_block.h>
#include <lac/analysis/user_defined.h>
#include <lac/completion/function_at_pos.h>
#include <lac/completion/type_at_pos.h>
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

		goToCallStart();
		if (!ps)
			return {};
		--ps;

		ignoreWhiteSpace();

		const auto info = getTypeAtPos(rootScope, view, ps);
		if (info.type != an::Type::function)
			return {};

		return ArgumentData{info, index};
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

		auto data = getArgumentAtPos(scope, program, 12); // (42
		REQUIRE(data.has_value());
		CHECK(data->first.type == an::Type::function);
		CHECK(data->first.function.parameters.size() == 1);
		CHECK(data->second == 0);

		data = getArgumentAtPos(scope, program, 25); // (x
		REQUIRE(data.has_value());
		CHECK(data->first.type == an::Type::function);
		CHECK(data->first.function.parameters.size() == 2);
		CHECK(data->second == 0);

		data = getArgumentAtPos(scope, program, 28); // (x, 10
		REQUIRE(data.has_value());
		CHECK(data->first.type == an::Type::function);
		CHECK(data->first.function.parameters.size() == 2);
		CHECK(data->second == 1);
	}
} // namespace lac::comp
