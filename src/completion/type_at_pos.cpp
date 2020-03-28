#include <analysis/visitor.h>
#include <completion/type_at_pos.h>
#include <completion/variable_at_pos.h>
#include <completion/get_block.h>
#include <parser/parser.h>

#include <doctest/doctest.h>

namespace
{
	lac::an::TypeInfo getType(const lac::an::Scope& scope, std::string_view name)
	{
		auto info = scope.getVariableType(name);
		if (info.type != lac::an::Type::nil)
			return info;

		return scope.getFunctionType(name);
	}
} // namespace

namespace lac::comp
{
	an::TypeInfo getTypeAtPos(std::string_view view, size_t pos)
	{
		// Parse the program
		ast::Block block;
		pos::Positions positions{view.begin(), view.end()};
		if (!parser::parseString(view, positions, block))
			return {};
		
		// Analyse the program
		auto scope = an::analyseBlock(block);
		return getTypeAtPos(scope, view, pos);
	}

	an::TypeInfo getTypeAtPos(const an::Scope& scope, std::string_view view, size_t pos)
	{
		// Parse what is under the cursor
		const auto var = parseVariableAtPos(view, pos);
		if (!var)
			return {};

		// Get the scope under the cursor
		auto ptr = pos::getScopeAtPos(scope, pos);
		if (!ptr)
			return {};

		// Iterate over the expression
		if (var->variable.start.get().type() != typeid(std::string))
			return {};
		auto type = getType(*ptr, boost::get<std::string>(var->variable.start));

		if (!var->variable.rest.empty())
		{
			for (const auto& r : var->variable.rest)
			{
				if (r.get().type() != typeid(ast::TableIndexName))
					return an::Type::unknown;

				type = type.member(boost::get<ast::TableIndexName>(r).name);
			}
		}

		if (var->member)
			type = type.member(var->member->name);

		return type;
	}

	TEST_SUITE_BEGIN("Type at position");

	TEST_CASE("Name only")
	{
		CHECK(getTypeAtPos("x = 42", 0).type == an::Type::number);
		CHECK(getTypeAtPos("x = 'test'", 0).type == an::Type::string);
		CHECK(getTypeAtPos("x = true", 0).type == an::Type::boolean);
		CHECK(getTypeAtPos("x = {}", 0).type == an::Type::table);

		CHECK(getTypeAtPos("local x", 6).type == an::Type::unknown);
		CHECK(getTypeAtPos("local x = 42", 6).type == an::Type::number);

		CHECK(getTypeAtPos("x = 42; function test() print(x) end", 30).type == an::Type::number);
		CHECK(getTypeAtPos("x = 'foo'; function test() print(x) end", 33).type == an::Type::string);
	}

	TEST_CASE("Member variable")
	{
		const std::string program = R"~~(
myTable = {}
myTable.num = 42
myTable.str = 'foobar'
myTable.test = false
myTable.func = function (a, b) return a + b end;
myTable.child = {}
myTabel.child.text = 'meow'
)~~";
		// Parse the program
		std::string_view view = program;
		ast::Block block;
		pos::Positions positions{view.begin(), view.end()};
		REQUIRE(parser::parseString(view, positions, block));

		// Analyse the program
		auto scope = an::analyseBlock(block);
		CHECK(getTypeAtPos(scope, view, 3).type == an::Type::table);
		CHECK(getTypeAtPos(scope, view, 23).type == an::Type::number);
		CHECK(getTypeAtPos(scope, view, 40).type == an::Type::string);
		CHECK(getTypeAtPos(scope, view, 63).type == an::Type::boolean);
		CHECK(getTypeAtPos(scope, view, 83).type == an::Type::function);
		CHECK(getTypeAtPos(scope, view, 133).type == an::Type::table);
		CHECK(getTypeAtPos(scope, view, 159).type == an::Type::string);
	}

	TEST_SUITE_END();
} // namespace lac::comp
