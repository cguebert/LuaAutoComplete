#include <analysis/visitor.h>
#include <completion/type_at_pos.h>
#include <completion/variable_at_pos.h>
#include <completion/get_block.h>
#include <parser/chunk.h>

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
		// Parse what is under the cursor
		const auto var = parseVariableAtPos(view, pos);
		if (!var)
			return {};

		// Parse the program
		ast::Block block;
		lac::pos::Positions positions{view.begin(), view.end()};
		if (!lac::parseString(view, positions, block))
			return {};

		// Get the scope under the cursor
		auto scope = an::analyseBlock(block);
		auto ptr = pos::getScopeAtPos(scope, pos);
		if (!ptr)
			return {};

		if (var->variable.start.get().type() != typeid(std::string))
			return {};
		auto type = getType(*ptr, boost::get<std::string>(var->variable.start));

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

	//	CHECK(getTypeAtPos("x = 42; function test() print(x) end", 30).type == an::Type::number);
	}

	TEST_SUITE_END();
} // namespace lac::comp
