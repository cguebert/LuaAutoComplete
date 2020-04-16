#include <lac/analysis/visitor.h>
#include <lac/completion/completion.h>
#include <lac/completion/get_block.h>
#include <lac/completion/variable_at_pos.h>
#include <lac/helper/test_utils.h>
#include <lac/parser/chunk.h>

#include <cctype>

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

		return *lhs == boost::get<ast::Variable>(rhs->start);
	}

	bool operator==(const boost::optional<ast::VariableOrFunction>& lhs, const boost::optional<ast::VariableOrFunction>& rhs)
	{
		if (!lhs && !rhs)
			return true;

		if (lhs.has_value() != rhs.has_value())
			return false;

		if (!(boost::get<ast::Variable>(lhs->start) == boost::get<ast::Variable>(rhs->start)))
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

		std::string str;
		if (var->start.get().type() == typeid(ast::Variable))
		{
			const auto& variable = boost::get<ast::Variable>(var->start);
			str += boost::get<std::string>(variable.start);
			for (const auto& r : variable.rest)
				str += '.' + boost::get<TableIndexName>(r).name;
		}
		else
		{
			const auto& variable = boost::get<ast::FunctionCall>(var->start);
			str += boost::get<std::string>(variable.start);
			for (const auto& r : variable.rest)
			{
				if (r.tableIndex)
					str += '.' + boost::get<TableIndexName>(*r.tableIndex).name;
				str += "()";
			}
		}

		if (var->member)
			str += ':' + var->member->name;

		return str.c_str();
	}
} // namespace lac::ast

namespace lac
{
	using helper::test_phrase_parser;

	namespace comp
	{
#ifndef DOCTEST_CONFIG_DISABLE
		const std::string program = R"~~(
function split_path(str)
	return split(str,'[\\/]+')
end

parts = split_path("/usr/local/bin")
print(parts)

local function myFunc(testValue)
	if (testValue % 2 == 0) then
		print('even')
	else
		print('odd')
	end

	x = testValue .. 'world'

    t = {}
    t.f = function (firstNum, secondNum) return firstNum + secondNum end
    t.m = testValue

    y = t.f(5, t.m)
end
)~~";

		std::string_view extractText(const ast::PositionAnnotated& pa)
		{
			auto ps = pa.begin, pe = pa.end;
			while (isspace(program[ps]))
				++ps;
			while (isspace(program[pe]))
				--pe;
			std::string_view view = program;
			return view.substr(ps, pe - ps + 1);
		}
#endif
		TEST_SUITE_BEGIN("Completion");

		TEST_CASE("Get block")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser(program, parser::chunkRule(), block));
			const auto blocks = pos::getChildren(block);

			auto ptr = pos::getBlockAtPos(blocks, 30);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(return split(str,'[\\/]+'))~");

			ptr = pos::getBlockAtPos(blocks, 180);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(print('even'))~");

			ptr = pos::getBlockAtPos(blocks, 202);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(print('odd'))~");

			ptr = pos::getBlockAtPos(blocks, 310);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(return firstNum + secondNum)~");
		}

		TEST_CASE("Get scope")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser(program, parser::chunkRule(), block));
			const auto scope = an::analyseBlock(block);

			auto ptr = pos::getScopeAtPos(scope, 30);
			REQUIRE(ptr);
			REQUIRE(ptr->block());
			CHECK(extractText(*ptr->block()) == R"~(return split(str,'[\\/]+'))~");

			ptr = pos::getScopeAtPos(scope, 180);
			REQUIRE(ptr);
			REQUIRE(ptr->block());
			CHECK(extractText(*ptr->block()) == R"~(print('even'))~");

			ptr = pos::getScopeAtPos(scope, 202);
			REQUIRE(ptr);
			REQUIRE(ptr->block());
			CHECK(extractText(*ptr->block()) == R"~(print('odd'))~");

			ptr = pos::getScopeAtPos(scope, 310);
			REQUIRE(ptr);
			REQUIRE(ptr->block());
			CHECK(extractText(*ptr->block()) == R"~(return firstNum + secondNum)~");
		}

		TEST_CASE("Scope elements")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser(program, parser::chunkRule(), block));
			const auto scope = an::analyseBlock(block);

			auto ptr = pos::getScopeAtPos(scope, 30);
			REQUIRE(ptr);
			auto elements = ptr->getElements();
			CHECK(elements.size() == 2);
			CHECK(elements.count("split_path"));
			CHECK(elements.count("str"));

			ptr = pos::getScopeAtPos(scope, 310);
			REQUIRE(ptr);
			elements = ptr->getElements();
			CHECK(elements.size() == 2);
			CHECK(elements.count("firstNum"));
			CHECK(elements.count("secondNum"));

			ptr = pos::getScopeAtPos(scope, 350);
			REQUIRE(ptr);
			elements = ptr->getElements();
			CHECK(elements.size() == 5);
			CHECK(elements.count("myFunc"));
			CHECK(elements.count("testValue"));
			CHECK(elements.count("x"));
			CHECK(elements.count("y"));
			REQUIRE(elements.count("t"));

			const auto& t = elements.at("t");
			CHECK(t.elementType == an::ElementType::variable);
			CHECK(t.typeInfo.type == an::Type::table);
			CHECK(t.typeInfo.member("f").type == an::Type::function);
			CHECK(t.typeInfo.member("m").type != an::Type::nil);
			CHECK(t.typeInfo.member("n").type == an::Type::nil);
		}

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
			ast::Block block;
			block.begin = 0;
			block.end = 42;

			an::Scope scope{block};
			scope.addVariable("num", an::Type::number);
			scope.addVariable("text", an::Type::string);
			scope.addVariable("bool", an::Type::boolean);
			scope.addVariable("func", an::Type::function);

			an::TypeInfo myTable{an::Type::table};
			myTable.members["memberNum"] = an::Type::number;
			myTable.members["memberText"] = an::Type::string;
			myTable.members["memberBool"] = an::Type::boolean;
			myTable.members["method1"] = an::TypeInfo::createMethod({});
			myTable.members["method2"] = an::TypeInfo::createMethod({});
			scope.addVariable("myTable", myTable);

			CHECK(getAutoCompletionList(scope, "").size() == 5);
			CHECK(getAutoCompletionList(scope, "none").size() == 5);
			CHECK(getAutoCompletionList(scope, "num").size() == 5);

			// A nil table has no members
			CHECK(getAutoCompletionList(scope, "noTable.test").size() == 0);
			CHECK(getAutoCompletionList(scope, "noTable:test").size() == 0);

			// Return all variables
			CHECK(getAutoCompletionList(scope, "myTable.").size() == 3);
			CHECK(getAutoCompletionList(scope, "myTable.memberNum").size() == 3);
			CHECK(getAutoCompletionList(scope, "myTable.dummy").size() == 3);

			// Return only methods
			CHECK(getAutoCompletionList(scope, "myTable:").size() == 2);
			CHECK(getAutoCompletionList(scope, "myTable:method1").size() == 2);
			CHECK(getAutoCompletionList(scope, "myTable:dummy").size() == 2);
		}

		TEST_CASE("Completion class")
		{
			std::string program = R"~~(
num = 42
text = 'foo'
test = true
func = function(a, b)
	return a + b
end

myTable = {}
myTable.m_num = 69
myTable.m_text = 'bar'
myTable.m_bool = false
myTable.m_func1 = function(a) return -a end
myTable.m_func2 = function(a) return a * 2 end
myTable.m_meth1 = function(self, a) return -a end
myTable.m_meth2 = function(self, a) return a * 2 end
)~~";

			Completion completion;
			REQUIRE(completion.updateProgram(program));

			CHECK(completion.getAutoCompletionList("").size() == 5);
			CHECK(completion.getAutoCompletionList("none").size() == 5);
			CHECK(completion.getAutoCompletionList("num").size() == 5);

			// Return all variables
			CHECK(completion.getAutoCompletionList("myTable.").size() == 5);
			CHECK(completion.getAutoCompletionList("myTable.memberNum").size() == 5);
			CHECK(completion.getAutoCompletionList("myTable.dummy").size() == 5);

			// Return only methods
			CHECK(completion.getAutoCompletionList("myTable:").size() == 2);
			CHECK(completion.getAutoCompletionList("myTable:method1").size() == 2);
			CHECK(completion.getAutoCompletionList("myTable:dummy").size() == 2);
		}

		TEST_CASE("Completion with errors")
		{
			// This program does not compile because of an error on line 2
			// but if we tell Completion that we are currently editing that line
			// it will remove it and the program can compile
			std::string program = R"~~(
function test(first, second)
	fir
end
)~~";

			Completion completion;
			REQUIRE_FALSE(completion.updateProgram(program));
			REQUIRE(completion.updateProgram(program, 32));

			auto list = completion.getAutoCompletionList(program, 32);
			CHECK(list.count("first") == 1);
		}

		TEST_CASE("Completion with user defined types")
		{
			using namespace lac::an;
			UserDefined userDefined;
			TypeInfo vec3Type = Type::table;
			vec3Type.name = "Vector3";
			vec3Type.members["x"] = Type::number;
			vec3Type.members["y"] = Type::number;
			vec3Type.members["z"] = Type::number;
			vec3Type.members["length"] = TypeInfo::createMethod({}, {Type::number});
			vec3Type.members["mult"] = TypeInfo::createMethod({{"v", Type::number}}, {TypeInfo::fromTypeName("Vector3")});
			userDefined.addType(std::move(vec3Type));

			TypeInfo playerType = Type::table;
			playerType.name = "Player";
			playerType.members["name"] = Type::string;
			playerType.members["id"] = TypeInfo::createMethod({}, {Type::number});
			playerType.members["position"] = TypeInfo::createMethod({}, {TypeInfo::fromTypeName("Vector3")});
			playerType.members["setPosition"] = TypeInfo::createMethod({{"position", TypeInfo::fromTypeName("Vector3")}}, {});
			userDefined.addType(std::move(playerType));

			userDefined.addScriptInput("run", {{{"player", TypeInfo::fromTypeName("Player")}}, {}});

			std::string program = R"~~(
function run(player)
	local len = player:position():length()
	if len > 500 then
		local newPos = player:position():mult(500 / len)
		player:setPosition(newPos)
	end
end
)~~";

			using StrVec = std::vector<std::string>;

			Completion completion;
			completion.setUserDefined(userDefined);
			REQUIRE(completion.updateProgram(program));

			auto list = completion.getAutoCompletionList(program, 41); // player:
			CHECK(list.size() == 3);
			CHECK(list.count("id"));
			CHECK(list.count("position"));
			CHECK(list.count("setPosition"));
			CHECK(completion.getTypeAtPos(program, 40) == playerType);
			CHECK(completion.getTypeHierarchyAtPos(program, 40) == StrVec{"Player"});

			list = completion.getAutoCompletionList(program, 44); // player:pos
			CHECK(list.size() == 3);
			CHECK(list.count("id"));
			CHECK(list.count("position"));
			CHECK(list.count("setPosition"));
			CHECK(completion.getTypeAtPos(program, 44).type == Type::function);
			CHECK(completion.getTypeHierarchyAtPos(program, 44) == StrVec{"Player", "position"});

			list = completion.getAutoCompletionList(program, 52); // player:position():
			CHECK(list.size() == 2);
			CHECK(list.count("length"));
			CHECK(list.count("mult"));

			list = completion.getAutoCompletionList(program, 55); // player:position():len
			CHECK(list.size() == 2);
			CHECK(list.count("length"));
			CHECK(list.count("mult"));
			CHECK(completion.getTypeAtPos(program, 55).type == Type::function);
			CHECK(completion.getTypeHierarchyAtPos(program, 55) == StrVec{"Vector3", "length"});
		}

		TEST_CASE("Completion of constructors")
		{
			using namespace lac::an;
			UserDefined userDefined;
			TypeInfo vec3Type = Type::table;
			vec3Type.name = "Vector3";
			vec3Type.members["new"] = TypeInfo::createFunction({}, {TypeInfo::fromTypeName("Vector3")});
			vec3Type.members["x"] = Type::number;
			vec3Type.members["y"] = Type::number;
			vec3Type.members["z"] = Type::number;
			vec3Type.members["length"] = TypeInfo::createMethod({}, {Type::number});
			vec3Type.members["mult"] = TypeInfo::createMethod({{"v", Type::number}}, {TypeInfo::fromTypeName("Vector3")});
			userDefined.addType(std::move(vec3Type));

			std::string program = R"~~(
vec = Vector3.new()
vec.x = 42
len = vec:length()
)~~";

			using StrVec = std::vector<std::string>;

			Completion completion;
			completion.setUserDefined(userDefined);
			REQUIRE(completion.updateProgram(program));

			auto list = completion.getAutoCompletionList(program, 15); // Vector3.n
			CHECK(list.size() == 1);
			CHECK(list.count("new"));
		}

		TEST_SUITE_END();
	} // namespace comp
} // namespace lac
