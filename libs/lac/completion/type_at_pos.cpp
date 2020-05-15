#include <lac/analysis/analyze_block.h>
#include <lac/analysis/get_type.h>
#include <lac/analysis/user_defined.h>
#include <lac/completion/type_at_pos.h>
#include <lac/completion/variable_at_pos.h>
#include <lac/completion/get_block.h>
#include <lac/parser/parser.h>

#include <doctest/doctest.h>

namespace
{
	lac::an::TypeInfo getType(const lac::an::Scope& scope, const std::string& name)
	{
		auto type = scope.getVariableType(name);
		if (!type)
		{
			type = scope.getUserType(name);
			// Only keep the static function members
			for (auto it = type.members.cbegin(); it != type.members.cend();)
			{
				const auto& mbr = it->second;
				if (mbr.type != lac::an::Type::function || mbr.function.isMethod)
					it = type.members.erase(it);
				else
					++it;
			}
		}
		return scope.resolve(type);
	}

	lac::an::TypeInfo processPostFix(const lac::an::Scope& scope, lac::an::TypeInfo type, const lac::ast::VariablePostfix& vpf)
	{
		const auto& vpfType = vpf.get().type();
		if (vpfType == typeid(lac::ast::TableIndexName))
			return scope.resolve(type.member(boost::get<lac::ast::TableIndexName>(vpf).name));
		if (vpfType == typeid(lac::ast::TableIndexExpression))
		{
			const auto tie = boost::get<lac::ast::TableIndexExpression>(vpf);
			if (type.type == lac::an::Type::array && lac::an::getType(scope, tie.expression).type == lac::an::Type::number)
				return scope.resolve(lac::an::TypeInfo::fromTypeName(type.name));
		}
		else if (vpfType == typeid(lac::ast::VariableFunctionCall))
		{
			const auto fc = boost::get<lac::ast::f_VariableFunctionCall>(vpf).get();
			const auto parent = type;
			if (fc.functionCall.member)
				type = type.member(*fc.functionCall.member);
			if (type.function.getResultTypeFunc)
				return type.function.getResultTypeFunc(scope, fc.functionCall.arguments, parent);
			if (type.function.results.empty())
				return lac::an::Type::unknown;
			type = scope.resolve(type.function.results.front());
			return processPostFix(scope, type, fc.postVariable);
		}

		return lac::an::Type::unknown;
	}

	lac::an::TypeInfo processPostFix(const lac::an::Scope& scope, lac::an::TypeInfo type, const lac::ast::FunctionCallPostfix& fcp)
	{
		if (fcp.tableIndex)
		{
			if (fcp.tableIndex->get().type() == typeid(lac::ast::TableIndexName))
				type = scope.resolve(type.member(boost::get<lac::ast::TableIndexName>(*fcp.tableIndex).name));
			else
			{
				const auto tie = boost::get<lac::ast::TableIndexExpression>(fcp.tableIndex->get());
				if (type.type == lac::an::Type::array && lac::an::getType(scope, tie.expression).type == lac::an::Type::number)
					return scope.resolve(lac::an::TypeInfo::fromTypeName(type.name));
				else
					return lac::an::Type::unknown;
			}
		}

		const auto parent = type;
		if (fcp.functionCall.member)
			type = type.member(*fcp.functionCall.member);
		if (type.function.getResultTypeFunc)
			return type.function.getResultTypeFunc(scope, fcp.functionCall.arguments, parent);
		if (type.function.results.empty())
			return lac::an::Type::unknown;
		return scope.resolve(type.function.results.front());
	}

	lac::an::TypeInfo getVariableType(const lac::an::Scope& localScope, const lac::ast::Variable& var)
	{
		if (var.start.get().type() != typeid(std::string))
			return {};

		auto type = getType(localScope, boost::get<std::string>(var.start));
		for (const auto& r : var.rest)
			type = processPostFix(localScope, type, r);

		return type;
	}

	lac::an::TypeInfo getFunctionCallType(const lac::an::Scope& localScope, const lac::ast::FunctionCall& fc)
	{
		if (fc.start.get().type() != typeid(std::string))
			return {};

		auto type = getType(localScope, boost::get<std::string>(fc.start));
		for (const auto& r : fc.rest)
			type = processPostFix(localScope, type, r);

		return type;
	}

	lac::an::TypeInfo processPostFix(std::vector<std::string>& hierarchy, const lac::an::Scope& scope, lac::an::TypeInfo type, const lac::ast::VariablePostfix& vpf)
	{
		if (vpf.get().type() == typeid(lac::ast::TableIndexName))
		{
			const auto name = boost::get<lac::ast::TableIndexName>(vpf).name;
			hierarchy.push_back(name);
			return scope.resolve(type.member(name));
		}
		else if (vpf.get().type() == typeid(lac::ast::VariableFunctionCall))
		{
			const auto fc = boost::get<lac::ast::f_VariableFunctionCall>(vpf).get();
			const auto parent = type;
			if (fc.functionCall.member)
				type = type.member(*fc.functionCall.member);
			if (type.function.getResultTypeFunc)
			{
				// Restart with the result of this call
				hierarchy.clear();
				const auto res = type.function.getResultTypeFunc(scope, fc.functionCall.arguments, parent);
				hierarchy.push_back(res.typeName());
				type = scope.resolve(res);
			}
			else
			{
				if (type.function.results.empty())
				{
					hierarchy.clear();
					return lac::an::Type::unknown;
				}

				// Restart with the result of this call
				hierarchy.clear();
				const auto& res = type.function.results.front();
				hierarchy.push_back(res.typeName());
				type = scope.resolve(res);
			}

			// Continue the chain
			return processPostFix(hierarchy, scope, type, fc.postVariable);
		}
		else
		{
			hierarchy.clear();
			return lac::an::Type::unknown;
		}
	}

	lac::an::TypeInfo processPostFix(std::vector<std::string>& hierarchy, const lac::an::Scope& scope, lac::an::TypeInfo type, const lac::ast::FunctionCallPostfix& fcp)
	{
		if (fcp.tableIndex)
		{
			if (fcp.tableIndex->get().type() == typeid(lac::ast::TableIndexName))
			{
				const auto name = boost::get<lac::ast::TableIndexName>(*fcp.tableIndex).name;
				hierarchy.push_back(name);
				type = scope.resolve(type.member(name));
			}
			else
			{
				hierarchy.clear();
				return lac::an::Type::unknown;
			}
		}

		const auto parent = type;
		if (fcp.functionCall.member)
			type = type.member(*fcp.functionCall.member);

		if (type.function.getResultTypeFunc)
		{
			// Restart with the result of this call
			hierarchy.clear();
			const auto res = type.function.getResultTypeFunc(scope, fcp.functionCall.arguments, parent);
			hierarchy.push_back(res.typeName());
			type = scope.resolve(res);
		}
		else
		{
			if (type.function.results.empty())
			{
				hierarchy.clear();
				return lac::an::Type::unknown;
			}

			// Restart with the result of this call
			hierarchy.clear();
			const auto& res = type.function.results.front();
			hierarchy.push_back(res.typeName());
			type = scope.resolve(res);
		}
		return type;
	}

	std::vector<std::string> getTypeHierarchy(const lac::an::Scope& localScope, const lac::ast::Variable& var)
	{
		if (var.start.get().type() != typeid(std::string))
			return {};

		const auto& name = boost::get<std::string>(var.start);

		auto type = localScope.getVariableType(name);
		if (!type)
			type = localScope.getUserType(name);

		std::vector<std::string> hierarchy;
		hierarchy.push_back(type.typeName()); // We want the type name before resolving userdata into table
		type = localScope.resolve(type);

		for (const auto& r : var.rest)
			type = processPostFix(hierarchy, localScope, type, r);
		return hierarchy;
	}

	std::vector<std::string> getTypeHierarchy(const lac::an::Scope& localScope, const lac::ast::FunctionCall& fc)
	{
		if (fc.start.get().type() != typeid(std::string))
			return {};
		const auto& name = boost::get<std::string>(fc.start);

		std::vector<std::string> hierarchy;
		auto type = localScope.getVariableType(name);
		if (type)
		{
			hierarchy.push_back(type.typeName()); // We want the type name before resolving userdata into table
			type = localScope.resolve(type);
		}

		for (const auto& r : fc.rest)
			type = processPostFix(hierarchy, localScope, type, r);
		return hierarchy;
	}
} // namespace

namespace lac::comp
{
	an::TypeInfo getTypeAtPos(std::string_view view, size_t pos)
	{
		if (pos == std::string_view::npos)
			pos = view.size() - 1;

		// Parse the program
		const auto ret = parser::parseBlock(view);
		if (!ret.parsed)
			return {};

		// Analyse the program
		auto scope = an::analyseBlock(ret.block);
		return getTypeAtPos(scope, view, pos);
	}

	an::TypeInfo getTypeAtPos(const an::Scope& rootScope, std::string_view view, size_t pos)
	{
		if (pos == std::string_view::npos)
			pos = view.size() - 1;

		// Parse what is under the cursor
		const auto var = parseVariableAtPos(view, pos);
		if (!var)
			return {};

		// Get the scope under the cursor
		auto scope = pos::getScopeAtPos(rootScope, pos);
		if (!scope)
			return {};

		return getVariableType(*scope, *var);
	}

	an::TypeInfo getVariableType(const an::Scope& localScope, const ast::VariableOrFunction& var)
	{
		an::TypeInfo type;
		if (var.start.get().type() == typeid(ast::Variable))
			type = ::getVariableType(localScope, boost::get<ast::Variable>(var.start));
		else
			type = ::getFunctionCallType(localScope, boost::get<ast::FunctionCall>(var.start));

		return var.member
				   ? localScope.resolve(type.member(var.member->name))
				   : type;
	}

	std::vector<std::string> getTypeHierarchyAtPos(const an::Scope& rootScope, std::string_view view, size_t pos)
	{
		if (pos == std::string_view::npos)
			pos = view.size() - 1;

		// Parse what is under the cursor
		const auto var = parseVariableAtPos(view, pos);
		if (!var)
			return {};

		// Get the scope under the cursor
		auto scope = pos::getScopeAtPos(rootScope, pos);
		if (!scope)
			return {};

		std::vector<std::string> hierarchy;
		if (var->start.get().type() == typeid(ast::Variable))
			hierarchy = getTypeHierarchy(*scope, boost::get<ast::Variable>(var->start));
		else
			hierarchy = getTypeHierarchy(*scope, boost::get<ast::FunctionCall>(var->start));

		if (var->member)
			hierarchy.push_back(var->member->name);
		return hierarchy;
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
			ast::Variable var;
			var.start = std::move(name);
			v.start = std::move(var);
			return v;
		};

		auto var2 = [](std::string name, std::string member) {
			ast::VariableOrFunction v;
			ast::Variable var;
			var.start = std::move(name);
			var.rest.push_back(ast::VariablePostfix{ast::TableIndexName{std::move(member)}});
			v.start = std::move(var);
			return v;
		};

		CHECK(getVariableType(scope, var("num")).type == an::Type::number);
		CHECK(getVariableType(scope, var("text")).type == an::Type::string);
		CHECK(getVariableType(scope, var("bool")).type == an::Type::boolean);
		CHECK(getVariableType(scope, var("myTable")).type == an::Type::table);
		CHECK(getVariableType(scope, var("none")).type == an::Type::nil);

		CHECK(getVariableType(scope, var2("myTable", "memberNum")).type == an::Type::number);
		CHECK(getVariableType(scope, var2("myTable", "memberText")).type == an::Type::string);
		CHECK(getVariableType(scope, var2("myTable", "memberBool")).type == an::Type::boolean);
		CHECK(getVariableType(scope, var2("myTable", "none")).type == an::Type::nil);
	}

	TEST_CASE("Name only")
	{
		CHECK(getTypeAtPos("x", 0).type == an::Type::nil);
		CHECK(getTypeAtPos("x = 42", 0).type == an::Type::number);
		CHECK(getTypeAtPos("x = 'test'", 0).type == an::Type::string);
		CHECK(getTypeAtPos("x = true", 0).type == an::Type::boolean);
		CHECK(getTypeAtPos("x = {}", 0).type == an::Type::table);

		CHECK(getTypeAtPos("local x", 6).type == an::Type::nil);
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
		const std::string_view view = program;
		auto scope = an::analyseBlock(ret.block);
		CHECK(getTypeAtPos(scope, view, 3).type == an::Type::table);
		CHECK(getTypeAtPos(scope, view, 23).type == an::Type::number);
		CHECK(getTypeAtPos(scope, view, 40).type == an::Type::string);
		CHECK(getTypeAtPos(scope, view, 63).type == an::Type::boolean);
		CHECK(getTypeAtPos(scope, view, 83).type == an::Type::function);
		CHECK(getTypeAtPos(scope, view, 133).type == an::Type::table);
		CHECK(getTypeAtPos(scope, view, 159).type == an::Type::string);
	}

	TEST_CASE("Method calls")
	{
		an::UserDefined userDefined;
		an::TypeInfo nodeType = an::Type::table;
		nodeType.name = "Node";
		const auto parentFuncType = an::TypeInfo::createFunction({}, {an::TypeInfo::fromTypeName("Node")});
		nodeType.members["parent"] = parentFuncType;
		const auto nbChildsFuncType = an::TypeInfo::createFunction({}, {an::Type::number});
		nodeType.members["nbChilds"] = nbChildsFuncType;
		const auto childFuncType = an::TypeInfo::createFunction({{"index", an::Type::number}}, {an::TypeInfo::fromTypeName("Node")});
		nodeType.members["child"] = childFuncType;
		userDefined.addType(std::move(nodeType));

		userDefined.addVariable("root", an::TypeInfo::createFunction({}, {an::TypeInfo::fromTypeName("Node")}));

		const std::string program = "root():child(1):child(2):parent():parent():nbChilds()";

		// Parse the program
		const auto ret = parser::parseBlock(program);
		REQUIRE(ret.parsed);

		// Analyse the program
		auto scope = an::Scope(ret.block);
		scope.setUserDefined(&userDefined);
		an::analyseBlock(scope, ret.block);

		const std::string_view view = program;
		using StrVec = std::vector<std::string>;

		CHECK(getTypeAtPos(scope, view, 10) == childFuncType);
		CHECK(getTypeAtPos(scope, view, 20) == childFuncType);
		CHECK(getTypeAtPos(scope, view, 30) == parentFuncType);
		CHECK(getTypeAtPos(scope, view, 38) == parentFuncType);
		CHECK(getTypeAtPos(scope, view, 50) == nbChildsFuncType);

		CHECK(getTypeHierarchyAtPos(scope, view, 10) == StrVec{"Node", "child"});
		CHECK(getTypeHierarchyAtPos(scope, view, 20) == StrVec{"Node", "child"});
		CHECK(getTypeHierarchyAtPos(scope, view, 30) == StrVec{"Node", "parent"});
		CHECK(getTypeHierarchyAtPos(scope, view, 38) == StrVec{"Node", "parent"});
		CHECK(getTypeHierarchyAtPos(scope, view, 50) == StrVec{"Node", "nbChilds"});
	}

	TEST_SUITE_END();
} // namespace lac::comp
