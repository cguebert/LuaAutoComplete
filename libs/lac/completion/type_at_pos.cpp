#include <lac/analysis/visitor.h>
#include <lac/completion/type_at_pos.h>
#include <lac/completion/variable_at_pos.h>
#include <lac/completion/get_block.h>
#include <lac/parser/parser.h>

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
		const auto ret = parser::parseBlock(view);
		if (!ret.parsed)
			return {};
		
		// Analyse the program
		auto scope = an::analyseBlock(ret.block);
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

		return getTypeAtPos(*ptr, *var);
	}

	an::TypeInfo getTypeAtPos(const an::Scope& scope, const ast::VariableOrFunction& var)
	{
		// Iterate over the expression
		if (var.variable.start.get().type() != typeid(std::string))
			return {};
		auto type = getType(scope, boost::get<std::string>(var.variable.start));

		if (!var.variable.rest.empty())
		{
			for (const auto& r : var.variable.rest)
			{
				if (r.get().type() != typeid(ast::TableIndexName))
					return an::Type::unknown;

				type = type.member(boost::get<ast::TableIndexName>(r).name);
			}
		}

		if (var.member)
			type = type.member(var.member->name);

		return type;
	}

	TEST_SUITE_BEGIN("Type at position");

	TEST_CASE("Raw structs")
	{
		an::Scope scope;
		scope.addVariable("num", an::Type::number);
		scope.addVariable("text", an::Type::string);
		scope.addVariable("bool", an::Type::boolean);

		an::TypeInfo myTable{an::Type::table};
		myTable.members["memberNum"] = an::Type::number;
		myTable.members["memberText"] = an::Type::string;
		myTable.members["memberBool"] = an::Type::boolean;
		scope.addVariable("myTable", myTable);

		auto var = [](std::string name) {
			ast::VariableOrFunction v;
			v.variable.start = std::move(name);
			return v;
		};

		auto var2 = [](std::string name, std::string member) {
			ast::VariableOrFunction v;
			v.variable.start = std::move(name);
			v.variable.rest.push_back(ast::VariablePostfix{ast::TableIndexName{std::move(member)}});
			return v;
		};
		
		CHECK(getTypeAtPos(scope, var("num")).type == an::Type::number);
		CHECK(getTypeAtPos(scope, var("text")).type == an::Type::string);
		CHECK(getTypeAtPos(scope, var("bool")).type == an::Type::boolean);
		CHECK(getTypeAtPos(scope, var("myTable")).type == an::Type::table);
		CHECK(getTypeAtPos(scope, var("none")).type == an::Type::nil);

		CHECK(getTypeAtPos(scope, var2("myTable", "memberNum")).type == an::Type::number);
		CHECK(getTypeAtPos(scope, var2("myTable", "memberText")).type == an::Type::string);
		CHECK(getTypeAtPos(scope, var2("myTable", "memberBool")).type == an::Type::boolean);
		CHECK(getTypeAtPos(scope, var2("myTable", "none")).type == an::Type::nil);
	}

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
		const auto ret = parser::parseBlock(program);
		REQUIRE(ret.parsed);

		// Analyse the program
		const auto view = program;
		auto scope = an::analyseBlock(ret.block);
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
