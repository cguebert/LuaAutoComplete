#include <lac/completion/completion.h>
#include <lac/completion/get_block.h>
#include <lac/completion/type_at_pos.h>
#include <lac/completion/variable_at_pos.h>
#include <lac/parser/chunk.h>
#include <lac/parser/parser.h>

#include <doctest/doctest.h>

namespace lac::ast
{
	// The next functions are incomplete and must only be used for the tests in this file
	bool operator==(const ast::Variable& lhs, const ast::Variable& rhs)
	{
		if (boost::get<std::string>(lhs.start) != boost::get<std::string>(rhs.start))
			return false;

		if (lhs.rest.size() != rhs.rest.size())
			return false;

		for (auto lIt = lhs.rest.begin(), rIt = rhs.rest.begin();
			 lIt != lhs.rest.end();
			 ++lIt, ++rIt)
		{
			if (boost::get<TableIndexName>(*lIt).name != boost::get<TableIndexName>(*rIt).name)
				return false;
		}

		return true;
	}

	bool operator==(const boost::optional<ast::Variable>& lhs, const boost::optional<ast::VariableOrFunction>& rhs)
	{
		if (!lhs && !rhs)
			return true;

		if (lhs.has_value() != rhs.has_value())
			return false;

		if (rhs->member)
			return false;

		return *lhs == rhs->variable;
	}

	bool operator==(const boost::optional<ast::VariableOrFunction>& lhs, const boost::optional<ast::VariableOrFunction>& rhs)
	{
		if (!lhs && !rhs)
			return true;

		if (lhs.has_value() != rhs.has_value())
			return false;

		if (!(lhs->variable == rhs->variable))
			return false;

		if (!lhs->member && !rhs->member)
			return true;

		if (lhs->member.has_value() != rhs->member.has_value())
			return false;

		if (lhs->member->name != rhs->member->name)
			return false;

		return true;
	}

	doctest::String toString(const boost::optional<ast::Variable>& var)
	{
		if (!var)
			return "{}";

		std::string str = boost::get<std::string>(var->start);
		for (const auto& r : var->rest)
			str += '.' + boost::get<TableIndexName>(r).name;

		return str.c_str();
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

	boost::optional<ast::Variable> removeLastPart(ast::VariableOrFunction var)
	{
		// TODO: what can we do if the start is a bracketed expression?
		if (var.variable.start.get().type() != typeid(std::string))
			return {};

		// If there is a member function, remove it and return the rest as is
		if (var.member)
			return var.variable;

		// Else we must remove the last part of the variable
		if (var.variable.rest.empty())
			return {}; // There is nothing left

		var.variable.rest.pop_back();
		return var.variable;
	}

	an::ElementsMap getAutoCompletionList(const an::Scope& localScope, const boost::optional<ast::Variable>& var)
	{
		if (!var)
			return localScope.getElements();

		const auto info = getVariableType(localScope, *var);
		if (info.type == an::Type::nil)
			return localScope.getElements();

		return getElements(info);
	}

	boost::optional<ast::Variable> getContext(std::string_view str, size_t pos)
	{
		if (pos == std::string_view::npos)
			pos = str.size() - 1;
		if (pos >= str.size())
			return {};
		if (str[pos] == '.' || str[pos] == ':')
		{
			auto var = parseVariableAtPos(str, pos - 1); // Do not remove the last part, as it does not exist
			if (var)
				return var->variable;
			return {};
		}

		auto var = parseVariableAtPos(str, pos);
		if (!var)
			return {};
		return removeLastPart(*var);
	};

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
		CHECK_FALSE(getContext("").has_value());
		CHECK_FALSE(getContext("var").has_value());
		CHECK(getContext("myTable.var") == parseVariableAtPos("myTable"));
		CHECK(getContext("myTable.var.another") == parseVariableAtPos("myTable.var"));
		CHECK(getContext("myTable.var:func") == parseVariableAtPos("myTable.var"));
	}

	TEST_CASE("getAutoCompletionList")
	{
		an::Scope scope;
		scope.addVariable("num", an::Type::number);
		scope.addVariable("text", an::Type::string);
		scope.addVariable("bool", an::Type::boolean);
		scope.addVariable("func", an::Type::function);

		an::TypeInfo myTable{an::Type::table};
		myTable.members["memberNum"] = an::Type::number;
		myTable.members["memberText"] = an::Type::string;
		myTable.members["memberBool"] = an::Type::boolean;
		myTable.members["memberFunc"] = an::Type::function;
		scope.addVariable("myTable", myTable);

		auto getList = [&scope](std::string_view str) {
			return getAutoCompletionList(scope, getContext(str));
		};

		CHECK(getList("").size() == 5);
		CHECK(getList("none").size() == 5);
		CHECK(getList("num").size() == 5);

		CHECK(getList("myTable.").size() == 4);
		CHECK(getList("myTable:").size() == 4);
		CHECK(getList("myTable.memberNum").size() == 4);
	}

	TEST_SUITE_END();
} // namespace lac::comp
