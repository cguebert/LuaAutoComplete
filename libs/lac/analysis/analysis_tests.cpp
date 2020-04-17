#include <lac/analysis/analyze_block.h>
#include <lac/analysis/scope.h>
#include <lac/analysis/user_defined.h>
#include <lac/parser/chunk.h>

#include <lac/helper/arguments.h>
#include <lac/helper/test_utils.h>

namespace lac
{
	using helper::test_phrase_parser;

	namespace an
	{
		doctest::String toString(const Type& value)
		{
			return std::to_string(static_cast<int>(value)).c_str();
		}

		void test_expression_type(const std::string& input, Type type)
		{
			ast::Block block;
			auto statement = "local x = " + input;
			REQUIRE(test_phrase_parser(statement, parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			CHECK(scope.getVariableType("x").type == type);
		}

#define EXPRESSION_TYPE(exp, type) \
	SUBCASE(exp) { test_expression_type(exp, type); }

		TEST_CASE("Expression type")
		{
			// One operand
			EXPRESSION_TYPE("nil", Type::nil);
			EXPRESSION_TYPE("true", Type::boolean);
			EXPRESSION_TYPE("42", Type::number);
			EXPRESSION_TYPE("'test'", Type::string);
			EXPRESSION_TYPE("{}", Type::table);
			EXPRESSION_TYPE("{x=1, 2}", Type::table);
			EXPRESSION_TYPE("function () end", Type::function);

			// Logical operators
			EXPRESSION_TYPE("not {}", Type::boolean);
			EXPRESSION_TYPE("1 < 2", Type::boolean);
			EXPRESSION_TYPE("'a' < 'b'", Type::boolean);
			EXPRESSION_TYPE("1 == {}", Type::boolean);

			// Binary operations
			EXPRESSION_TYPE("-3", Type::number);
			EXPRESSION_TYPE("42 + 3.14", Type::number);
			EXPRESSION_TYPE("42 + 3 / 2 * 5 // 4", Type::number);
			EXPRESSION_TYPE("'hello' .. ' world'", Type::string);
			EXPRESSION_TYPE("1 or 42", Type::number);

			// Unknown types (depend on the values)
			EXPRESSION_TYPE("1 or 'test'", Type::unknown);
			EXPRESSION_TYPE("'hello' and {1, 2}", Type::unknown);

			// Conversions
			EXPRESSION_TYPE("42 + '3.14'", Type::number);
			EXPRESSION_TYPE("'test' .. 2", Type::string);

			// Errors
			EXPRESSION_TYPE("-{}", Type::error);
			EXPRESSION_TYPE("#42", Type::error);
			EXPRESSION_TYPE("1 + {}", Type::error);
			EXPRESSION_TYPE("1 + function() end", Type::error);
			EXPRESSION_TYPE("2 > 'a'", Type::error);
		}

		TEST_CASE("Simple assignment")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("x = 42", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("x");
			CHECK(info.type == Type::number);
		}

		TEST_CASE("Sequential assignments")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("x = 42; y = x", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto infoX = scope.getVariableType("x");
			CHECK(infoX.type == Type::number);
			const auto infoY = scope.getVariableType("y");
			CHECK(infoY.type == Type::number);
		}

		TEST_CASE("Multiple assignments")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("x, y, z = 42, true, 'test'", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			CHECK(scope.getVariableType("x").type == Type::number);
			CHECK(scope.getVariableType("y").type == Type::boolean);
			CHECK(scope.getVariableType("z").type == Type::string);
		}

		TEST_CASE("Function assignment")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("local x = function(x, y, z) end", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("x");
			REQUIRE(info.type == Type::function);
			const auto& fd = info.function;
			REQUIRE(fd.parameters.size() == 3);
			CHECK(fd.parameters[0].name() == "x");
			CHECK(fd.parameters[0].type().type == Type::unknown);
			CHECK(fd.parameters[1].name() == "y");
			CHECK(fd.parameters[2].name() == "z");
		}

		TEST_CASE("Function definition")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("function func(x, y) end", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("func");
			REQUIRE(info.type == Type::function);
			const auto& fd = info.function;
			CHECK_FALSE(fd.isMethod);
			REQUIRE(fd.parameters.size() == 2);
			CHECK(fd.parameters[0].name() == "x");
			CHECK(fd.parameters[0].type().type == Type::unknown);
			CHECK(fd.parameters[1].name() == "y");
		}

		TEST_CASE("Method definition")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("function func(self, x, y) end", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("func");
			REQUIRE(info.type == Type::function);
			const auto& fd = info.function;
			CHECK(fd.isMethod);
			REQUIRE(fd.parameters.size() == 2);
			CHECK(fd.parameters[0].name() == "x");
			CHECK(fd.parameters[0].type().type == Type::unknown);
			CHECK(fd.parameters[1].name() == "y");
		}

		TEST_CASE("Function scope")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("function func(x, y) return x + y end", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			REQUIRE(scope.children().size() == 1);
			const auto& child = scope.children().front();
			CHECK(child.getVariableType("x").type == Type::unknown);
			CHECK(child.getVariableType("func").type == Type::function); // A function is known inside itself (for recursive functions)
		}

		TEST_CASE("Local function definition")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("local function func(x, y) end", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("func");
			REQUIRE(info.type == Type::function);
			const auto& fd = info.function;
			REQUIRE(fd.parameters.size() == 2);
			CHECK(fd.parameters[0].name() == "x");
			CHECK(fd.parameters[0].type().type == Type::unknown);
			CHECK(fd.parameters[1].name() == "y");
		}

		TEST_CASE("Table constructor by assignement")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("t = {x=42, name='foo', test=false}", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("t");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 3);

			REQUIRE(info.hasMember("x"));
			CHECK(info.member("x").type == Type::number);

			REQUIRE(info.hasMember("name"));
			CHECK(info.member("name").type == Type::string);

			REQUIRE(info.hasMember("test"));
			CHECK(info.member("test").type == Type::boolean);
		}

		TEST_CASE("Table constructor expression only")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("t = {42, 'test', false}", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("t");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 3);

			REQUIRE(info.hasMember("1"));
			CHECK(info.member("1").type == Type::number);

			REQUIRE(info.hasMember("2"));
			CHECK(info.member("2").type == Type::string);

			REQUIRE(info.hasMember("3"));
			CHECK(info.member("3").type == Type::boolean);
		}

		TEST_CASE("Table constructor varied")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("t = {1, text='foo', false}", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("t");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 3);

			REQUIRE(info.hasMember("1"));
			CHECK(info.member("1").type == Type::number);

			REQUIRE(info.hasMember("2"));
			CHECK(info.member("2").type == Type::boolean);

			REQUIRE(info.hasMember("text"));
			CHECK(info.member("text").type == Type::string);

			CHECK_FALSE(info.hasMember("3"));
		}

		TEST_CASE("Table member assignment")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("t = {}; t.x=42; t.str='foo'; t.test=true", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("t");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 3);

			REQUIRE(info.hasMember("x"));
			CHECK(info.member("x").type == Type::number);

			REQUIRE(info.hasMember("str"));
			CHECK(info.member("str").type == Type::string);

			REQUIRE(info.hasMember("test"));
			CHECK(info.member("test").type == Type::boolean);
		}

		TEST_CASE("Table member hierarchy")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser("a={}; a.b={}; a.b.v=42; a.b.c={}; a.b.c.t=true", parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("a");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 1);

			CHECK(info.member("b").type == Type::table);
			CHECK(info.member("b").member("v").type == Type::number);
			CHECK(info.member("b").member("c").type == Type::table);
			CHECK(info.member("b").member("c").member("t").type == Type::boolean);
		}

		TEST_CASE("Table function definition syntactic sugar")
		{
			ast::Block block;
			const auto program = R"~~(
t = {}
t.val = 42
function t.func(x)
	return -x;
end

function t:getVal()
	return self.val
end

function t.setVal(self, x)
	self.val = x
end
)~~";
			REQUIRE(test_phrase_parser(program, parser::chunkRule(), block));

			auto scope = analyseBlock(block);
			const auto info = scope.getVariableType("t");
			REQUIRE(info.type == Type::table);
			REQUIRE(info.members.size() == 4);
			CHECK(info.member("val").type == Type::number);

			const auto func = info.member("func");
			CHECK(func.type == Type::function);
			CHECK_FALSE(func.isMethod());
			REQUIRE(func.function.parameters.size() == 1);
			CHECK(func.function.parameters.front().name() == "x");

			const auto getVal = info.member("getVal");
			CHECK(getVal.type == Type::function);
			CHECK(getVal.isMethod());
			CHECK(getVal.function.parameters.empty());

			const auto setVal = info.member("setVal");
			CHECK(setVal.type == Type::function);
			CHECK(setVal.isMethod());
			REQUIRE(setVal.function.parameters.size() == 1);
			CHECK(setVal.function.parameters.front().name() == "x");
		}

		TEST_SUITE_BEGIN("User defined");

		TEST_CASE("Variables")
		{
			UserDefined user;
			user.addVariable("x", Type::number);
			user.addVariable("y", Type::boolean);

			Scope scope;
			scope.setUserDefined(&user);

			CHECK(scope.getVariableType("x").type == Type::number);
			CHECK(scope.getVariableType("y").type == Type::boolean);
		}

		TEST_CASE("Functions")
		{
			UserDefined user;
			user.addVariable("func", TypeInfo::createFunction({{"x", Type::number},
															   {"y"}},
															  {Type::number}));

			Scope scope;
			scope.setUserDefined(&user);

			const auto func = scope.getVariableType("func");
			REQUIRE(func.type == Type::function);
			REQUIRE(func.function.results.size() == 1);
			CHECK(func.function.results[0].type == Type::number);
			REQUIRE(func.function.parameters.size() == 2);
			CHECK(func.function.parameters[0].name() == "x");
			CHECK(func.function.parameters[0].type().type == Type::number);
			CHECK(func.function.parameters[1].name() == "y");
			CHECK(func.function.parameters[1].type().type == Type::unknown);
		}

		TEST_CASE("Type")
		{
			UserDefined user;

			TypeInfo complexType = Type::table;
			complexType.name = "Complex";
			complexType.members["new"] = TypeInfo::createFunction({{"real", Type::number},
																   {"imag", Type::number}},
																  {TypeInfo::fromTypeName("Complex")});
			complexType.members["real"] = Type::number;
			complexType.members["imag"] = Type::number;
			user.addType(complexType);

			user.addVariable("createComplex", TypeInfo::createFunction({{"real", Type::number},
																		{"imag", Type::number}},
																	   {TypeInfo::fromTypeName("Complex")}));

			user.addScriptInput("run", {{{"num", Type::number},
										 {"name", Type::string}}});

			Scope parentScope;
			parentScope.setUserDefined(&user);

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = createComplex(0.7, 1.0)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::userdata);
				CHECK(info.name == "Complex");
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = Complex.new(0.7, 1.0)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::userdata);
				CHECK(info.name == "Complex");
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = createComplex(0.7, 1.0).real", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::number);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = createComplex(0.7, 1.0); local y = x.imag", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto infoX = scope.getVariableType("x");
				CHECK(infoX.type == Type::userdata);
				CHECK(infoX.name == "Complex");
				const auto type = scope.getUserType(infoX.name);
				CHECK(type.members.size() == 3);
				CHECK(type.member("new").type == Type::function);
				CHECK(type.member("real").type == Type::number);
				CHECK(type.member("imag").type == Type::number);
				CHECK(type.member("xxx").type == Type::nil);
				const auto infoY = scope.getVariableType("y");
				CHECK(infoY.type == Type::number);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("function run(i, n) print(n) end", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				REQUIRE(scope.children().size() == 1);
				const auto& funcScope = scope.children().front();
				const auto infoI = funcScope.getVariableType("i");
				CHECK(infoI.type == Type::number);
				const auto infoN = funcScope.getVariableType("n");
				CHECK(infoN.type == Type::string);
			}
		}

		TEST_CASE("User defined multi types")
		{
			UserDefined userDefined;
			TypeInfo vec3Type = Type::table;
			vec3Type.name = "Vector3";
			vec3Type.members["x"] = Type::number;
			vec3Type.members["y"] = Type::number;
			vec3Type.members["z"] = Type::number;
			vec3Type.members["length"] = TypeInfo::createFunction({}, {Type::number});
			vec3Type.members["mult"] = TypeInfo::createFunction({{"v", Type::number}}, {TypeInfo::fromTypeName("Vector3")});
			userDefined.addType(std::move(vec3Type));

			TypeInfo playerType = Type::table;
			playerType.name = "Player";
			playerType.members["name"] = Type::string;
			playerType.members["id"] = TypeInfo::createFunction({}, {Type::number});
			playerType.members["position"] = TypeInfo::createFunction({}, {TypeInfo::fromTypeName("Vector3")});
			playerType.members["setPosition"] = TypeInfo::createFunction({{"position", TypeInfo::fromTypeName("Vector3")}}, {});
			userDefined.addType(std::move(playerType));

			userDefined.addVariable("getPlayer", TypeInfo::createFunction({}, {TypeInfo::fromTypeName("Player")}));

			Scope parentScope;
			parentScope.setUserDefined(&userDefined);

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local player = getPlayer()", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("player");
				CHECK(info.type == Type::userdata);
				CHECK(info.name == "Player");
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local name = getPlayer().name", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("name");
				CHECK(info.type == Type::string);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local pos = getPlayer():position()", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("pos");
				CHECK(info.type == Type::userdata);
				CHECK(info.name == "Vector3");
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local len = getPlayer():position():length()", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("len");
				CHECK(info.type == Type::number);
			}
		}

		TEST_CASE("Function return type callback")
		{
			auto testCallback = [](const an::Scope&, const ast::Arguments& args) -> TypeInfo {
				const auto str = helper::getLiteralString(args);
				if (!str)
					return Type::unknown;

				const auto& name = *str;
				if (name == "int")
					return Type::number;
				else if (name == "string")
					return Type::string;
				else if (name == "bool")
					return Type::boolean;

				return Type::unknown;
			};

			UserDefined userDefined;
			userDefined.addVariable("create", TypeInfo::createFunction({{"type", Type::string}}, {}, testCallback));

			Scope parentScope;
			parentScope.setUserDefined(&userDefined);

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = create('int')", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::number);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = create('string')", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::string);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = create('bool')", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::boolean);
			}
		}

		TEST_CASE("Function callback argument type")
		{
			auto testCallback = [](const an::Scope& scope, const ast::Arguments& args) -> TypeInfo {
				return helper::getType(scope, args, 0);
			};

			UserDefined userDefined;
			userDefined.addVariable("test", TypeInfo::createFunction({{"arg", Type::unknown}}, {}, testCallback));

			Scope parentScope;
			parentScope.setUserDefined(&userDefined);

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = test(42)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::number);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = test('foo')", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::string);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local x = test(true)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::boolean);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local y = 42; local x = test(y)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::number);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local y = 'foo'; local x = test(y)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::string);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local y = false; local x = test(y)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("x");
				CHECK(info.type == Type::boolean);
			}
		}

		TEST_CASE("Function callback custom data")
		{
			// This creates a type containing a custom data
			auto createToken = [](const an::Scope& scope, const ast::Arguments& args) -> TypeInfo {
				const auto str = helper::getLiteralString(args, 0);
				if (!str)
					return Type::unknown;

				auto info = TypeInfo::fromTypeName("Token");
				info.custom = *str;
				return info;
			};

			// This creates a table based on the custom data contained in the argument
			auto createObject = [](const an::Scope& scope, const ast::Arguments& args) -> TypeInfo {
				auto token = helper::getType(scope, args, 0);
				if (token.name != "Token" || token.custom.type() != typeid(std::string))
					return Type::unknown;

				TypeInfo out = Type::table;
				const auto type = std::any_cast<std::string>(token.custom);
				if (type == "cat")
					out.members["meow"] = TypeInfo::createMethod({}, {});
				else if (type == "dog")
					out.members["bark"] = TypeInfo::createMethod({}, {});
				else
					return Type::unknown;
				return out;
			};

			UserDefined userDefined;
			userDefined.addVariable("token", TypeInfo::createFunction({{"type", Type::string}}, {TypeInfo::fromTypeName("Token")}, createToken));
			userDefined.addVariable("create", TypeInfo::createFunction({{"token", TypeInfo::fromTypeName("Token")}}, {Type::table}, createObject));

			Scope parentScope;
			parentScope.setUserDefined(&userDefined);

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local t = token('foo')", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("t");
				CHECK(info.type == Type::userdata);
				CHECK(info.name == "Token");
				REQUIRE(info.custom.type() == typeid(std::string));
				CHECK(std::any_cast<std::string>(info.custom) == "foo");
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local o = create('dog')", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("o");
				CHECK(info.type == Type::unknown);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local t = token('foo'); local o = create(t)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("o");
				CHECK(info.type == Type::unknown);
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local t = token('dog'); local o = create(t)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("o");
				CHECK(info.type == Type::table);
				CHECK(info.hasMember("bark"));
				CHECK(info.member("bark").type == Type::function);
				CHECK(info.member("bark").function.isMethod);
				CHECK_FALSE(info.hasMember("meow"));
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local t = token('cat'); local o = create(t)", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("o");
				CHECK(info.type == Type::table);
				CHECK(info.hasMember("meow"));
				CHECK(info.member("meow").type == Type::function);
				CHECK(info.member("meow").function.isMethod);
				CHECK_FALSE(info.hasMember("bark"));
			}

			{
				ast::Block block;
				REQUIRE(test_phrase_parser("local o = create(token('dog'))", parser::chunkRule(), block));
				const auto scope = analyseBlock(block, &parentScope);
				const auto info = scope.getVariableType("o");
				CHECK(info.type == Type::table);
				CHECK(info.hasMember("bark"));
				CHECK(info.member("bark").type == Type::function);
				CHECK(info.member("bark").function.isMethod);
				CHECK_FALSE(info.hasMember("meow"));
			}
		}

		TEST_SUITE_END();
	} // namespace an
} // namespace lac
