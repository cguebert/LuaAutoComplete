#include <lac/completion/completion.h>
#include <lac/completion/get_block.h>
#include <lac/completion/variable_at_pos.h>
#include <lac/parser/parser.h>

#include <doctest/doctest.h>

namespace lac::ast
{
	// The next 2 functions are incomplete and must only be used for the tests in this file
	bool operator==(const boost::optional<ast::VariableOrFunction>& lhs, const boost::optional<ast::VariableOrFunction>& rhs)
	{
		if (!lhs && !rhs)
			return true;

		if (lhs.has_value() != rhs.has_value())
			return false;

		if (boost::get<std::string>(lhs->variable.start) != boost::get<std::string>(rhs->variable.start))
			return false;

		if (lhs->variable.rest.size() != rhs->variable.rest.size())
			return false;

		for (auto lIt = lhs->variable.rest.begin(), rIt = rhs->variable.rest.begin();
			 lIt != lhs->variable.rest.end();
			 ++lIt, ++rIt)
		{
			if (boost::get<TableIndexName>(*lIt).name != boost::get<TableIndexName>(*rIt).name)
				return false;
		}

		if (!lhs->member && !rhs->member)
			return true;

		if (lhs->member.has_value() != rhs->member.has_value())
			return false;

		if (lhs->member->name != rhs->member->name)
			return false;

		return true;
	}

	doctest::String toString(const boost::optional<ast::VariableOrFunction>& var)
	{
		if (!var)
			return "{}";

		std::string str = boost::get<std::string>(var->variable.start);
		for (const auto& r : var->variable.rest)
			str += '.' + boost::get<TableIndexName>(r).name;

		if (var->member)
			str += ':' + var->member->name;

		return str.c_str();
	}
} // namespace lac::ast

namespace lac::comp
{
	bool Completion::updateProgram(std::string_view view, size_t currentPosition)
	{
		auto ret = lac::parser::parseBlock(view);

		if (ret.parsed)
		{
			std::swap(m_rootBlock, ret.block);
			std::swap(m_positions, ret.positions);
		}

		return ret.parsed;
	}

	boost::optional<ast::VariableOrFunction> removeLastPart(ast::VariableOrFunction var)
	{
		// TODO: what can we do if the start is a bracketed expression?
		if (var.variable.start.get().type() != typeid(std::string))
			return {};

		// If there is a member function, remove it and return the rest as is
		if (var.member)
		{
			var.member.reset();
			return var;
		}

		// Else we must remove the last part of the variable
		if (var.variable.rest.empty())
			return {}; // There is nothing left

		var.variable.rest.pop_back();
		return var;
	}

	TEST_SUITE_BEGIN("Completion");

	TEST_CASE("VariableOrFunction comparison")
	{
		CHECK(parseVariableAtPos("var") == parseVariableAtPos("var"));
		CHECK(parseVariableAtPos("var") != parseVariableAtPos("test"));
		CHECK(parseVariableAtPos("myTable.member") == parseVariableAtPos("myTable.member"));
		CHECK(parseVariableAtPos("myTable.member") != parseVariableAtPos("myTable.another"));
		CHECK(parseVariableAtPos("myTable:func") == parseVariableAtPos("myTable:func"));
		CHECK(parseVariableAtPos("myTable:func") != parseVariableAtPos("myTable:another"));
		CHECK(parseVariableAtPos("myTable:func") != parseVariableAtPos("myTable.another"));
		CHECK(parseVariableAtPos("myTable.var:func") == parseVariableAtPos("myTable.var:func"));
		CHECK(parseVariableAtPos("myTable.var:func") != parseVariableAtPos("myTable.var:another"));
		CHECK(parseVariableAtPos("myTable.var:func") != parseVariableAtPos("myTable.var.another"));
	}

	TEST_CASE("removeLastPart")
	{
		auto getContext = [](std::string_view str) -> boost::optional<ast::VariableOrFunction> {
			auto var = parseVariableAtPos(str);
			if (!var)
				return {};
			return removeLastPart(*var);
		};

		CHECK_FALSE(getContext("").has_value());
		CHECK_FALSE(getContext("var").has_value());
		CHECK(getContext("myTable.var") == parseVariableAtPos("myTable"));
		CHECK(getContext("myTable.var.another") == parseVariableAtPos("myTable.var"));
		CHECK(getContext("myTable.var:func") == parseVariableAtPos("myTable.var"));
	}

	TEST_SUITE_END();
} // namespace lac::comp
