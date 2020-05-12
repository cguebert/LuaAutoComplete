#include <lac/analysis/type_info.h>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <boost/optional.hpp>

#include <doctest/doctest.h>

namespace lac::type::ast
{
	struct NamedType
	{
		std::string typeName;
		bool isArray = false;
	};

	struct Argument
	{
		NamedType type;
		std::string name;
	};

	struct FunctionType
	{
		std::vector<NamedType> results;
		bool isMethod = false;
		std::vector<Argument> arguments;
	};

	struct ParsedType : boost::spirit::x3::variant<NamedType, FunctionType>
	{
		using base_type::base_type;
		using base_type::operator=;
	};
} // namespace lac::type::ast

BOOST_FUSION_ADAPT_STRUCT(lac::type::ast::NamedType, typeName, isArray)
BOOST_FUSION_ADAPT_STRUCT(lac::type::ast::Argument, type, name)
BOOST_FUSION_ADAPT_STRUCT(lac::type::ast::FunctionType, results, isMethod, arguments)

namespace lac::type::parser
{
	namespace x3 = boost::spirit::x3;
	namespace ascii = boost::spirit::x3::ascii;

	using ascii::char_;
	using x3::alnum;
	using x3::alpha;
	using x3::lexeme;
	using x3::lit;

#define RULE(name, type) \
	struct name          \
	{                    \
	};                   \
	const x3::rule<struct name, type> name = #name;

	RULE(name, std::string)
	RULE(namedType, ast::NamedType)
	RULE(namedTypes, std::vector<ast::NamedType>)
	RULE(argument, ast::Argument)
	RULE(function, ast::FunctionType)
	RULE(parsedType, ast::ParsedType)

	const auto name_def = lexeme[((alpha | char_('_'))
								  >> *(alnum | char_('_')))
								 - (lit("function") | lit("method"))];

	const auto namedType_def = name >> -(lit("[]") >> x3::attr(true));
	const auto namedTypes_def = -(namedType % ',');

	const auto argument_def = namedType >> name;

	const auto function_def = namedTypes
							  >> ((lit("function") >> x3::attr(false))
								  | (lit("method") >> x3::attr(true)))
							  >> '('
							  >> -(argument % ',')
							  >> ')';

	const auto parsedType_def = function | namedType;

	BOOST_SPIRIT_DEFINE(name, namedType, namedTypes, argument, function, parsedType)

	// Attributes are given by the caller
	template <class P, class... Args>
	bool test_parser(std::string_view input, const P& parser, Args&... args)
	{
		auto f = input.begin();
		const auto l = input.end();
		return boost::spirit::x3::phrase_parse(f, l, parser, boost::spirit::x3::ascii::space, args...) && f == l;
	}

	// A dummy attribute is added
	template <class P>
	bool test_parser(std::string_view input, const P& parser)
	{
		auto f = input.begin();
		const auto l = input.end();
		typename P::attribute_type val;
		return boost::spirit::x3::phrase_parse(f, l, parser, boost::spirit::x3::ascii::space, val) && f == l;
	}

	// Test the result of the parser against the given value
	template <class P, class V>
	bool test_value(std::string_view input, const P& parser, const V& value)
	{
		auto f = input.begin();
		const auto l = input.end();
		V val;
		return boost::spirit::x3::phrase_parse(f, l, parser, boost::spirit::x3::ascii::space, val)
			   && f == l
			   && val == value;
	}

	TEST_CASE("typeName")
	{
		CHECK(test_parser("test", name));
		CHECK(test_parser("test_123", name));

		CHECK_FALSE(test_parser("123test", name));
		CHECK_FALSE(test_parser("test 123", name));
		CHECK_FALSE(test_parser("function", name));
		CHECK_FALSE(test_parser("method", name));
	}

	TEST_CASE("NamedType")
	{
		CHECK(test_parser("int", namedType));
		CHECK(test_parser("int[]", namedType));
		CHECK(test_parser("int []", namedType));

		CHECK_FALSE(test_parser("[]", namedType));

		{
			ast::NamedType val;
			CHECK(test_parser("int", namedType, val));
			CHECK(val.typeName == "int");
			CHECK(val.isArray == false);
		}

		{
			ast::NamedType val;
			CHECK(test_parser("string", namedType, val));
			CHECK(val.typeName == "string");
			CHECK(val.isArray == false);
		}

		{
			ast::NamedType val;
			CHECK(test_parser("int[]", namedType, val));
			CHECK(val.typeName == "int");
			CHECK(val.isArray == true);
		}

		{
			ast::NamedType val;
			CHECK(test_parser("string[]", namedType, val));
			CHECK(val.typeName == "string");
			CHECK(val.isArray == true);
		}

		{
			ast::NamedType val;
			CHECK(test_parser("Player[]", namedType, val));
			CHECK(val.typeName == "Player");
			CHECK(val.isArray == true);
		}
	}

	TEST_CASE("NamedTypes")
	{
		CHECK(test_parser("", namedTypes));
		CHECK(test_parser("int", namedTypes));
		CHECK(test_parser("int, string", namedTypes));
		CHECK(test_parser("int[], string", namedTypes));

		CHECK_FALSE(test_parser(", ", namedTypes));

		{
			std::vector<ast::NamedType> val;
			CHECK(test_parser("", namedTypes, val));
			CHECK(val.empty());
		}

		{
			std::vector<ast::NamedType> val;
			CHECK(test_parser("int[], string", namedTypes, val));
			REQUIRE(val.size() == 2);
			CHECK(val[0].typeName == "int");
			CHECK(val[0].isArray);
			CHECK(val[1].typeName == "string");
			CHECK(val[1].isArray == false);
		}
	}

	TEST_CASE("Argument")
	{
		CHECK(test_parser("int x", argument));
		CHECK(test_parser("int[] x", argument));
		CHECK(test_parser("int [] x", argument));

		CHECK_FALSE(test_parser("[] x", argument));
		CHECK_FALSE(test_parser("test", argument));
		CHECK_FALSE(test_parser("test []", argument));

		{
			ast::Argument arg;
			CHECK(test_parser("int num", argument, arg));
			CHECK(arg.type.typeName == "int");
			CHECK(arg.type.isArray == false);
			CHECK(arg.name == "num");
		}

		{
			ast::Argument arg;
			CHECK(test_parser("Player[] player", argument, arg));
			CHECK(arg.type.typeName == "Player");
			CHECK(arg.type.isArray == true);
			CHECK(arg.name == "player");
		}
	}

	TEST_CASE("Function")
	{
		CHECK(test_parser("function()", function));
		CHECK(test_parser("function(boolean b)", function));
		CHECK(test_parser("number function()", function));
		CHECK(test_parser("number function(string name, boolean b)", function));
		CHECK(test_parser("string, number function(string name, boolean b)", function));

		CHECK(test_parser("string, number method(string name, boolean b)", function));

		{
			ast::FunctionType func;
			CHECK(test_parser("function()", function, func));
			CHECK(func.results.empty());
			CHECK(func.isMethod == false);
			CHECK(func.arguments.empty());
		}

		{
			ast::FunctionType func;
			CHECK(test_parser("string[], number function(string[] name, boolean b)", function, func));
			REQUIRE(func.results.size() == 2);
			CHECK(func.results[0].typeName == "string");
			CHECK(func.results[0].isArray);
			CHECK(func.results[1].typeName == "number");
			CHECK_FALSE(func.results[1].isArray);
			CHECK(func.isMethod == false);
			REQUIRE(func.arguments.size() == 2);
			CHECK(func.arguments[0].type.typeName == "string");
			CHECK(func.arguments[0].type.isArray);
			CHECK(func.arguments[1].type.typeName == "boolean");
			CHECK_FALSE(func.arguments[1].type.isArray);
		}
	}

	TEST_CASE("Parsed type")
	{
		CHECK(test_parser("number", parsedType));
		CHECK(test_parser("function()", parsedType));
		CHECK(test_parser("string function()", parsedType));
		CHECK(test_parser("number method(boolean b)", parsedType));
	}

} // namespace lac::type::parser

namespace lac::an
{
	TypeInfo createType(const type::ast::NamedType& named)
	{
		TypeInfo info;
		if (named.isArray)
		{
			info.type = Type::array;
			info.name = named.typeName;
		}
		else
			info = TypeInfo::fromTypeName(named.typeName);
		return info;
	}

	FunctionInfo createFunction(const type::ast::FunctionType& func)
	{
		FunctionInfo info;
		info.isMethod = func.isMethod;
		for (const auto& arg : func.arguments)
			info.parameters.emplace_back(arg.name, createType(arg.type));

		for (const auto& res : func.results)
			info.results.push_back(createType(res));
		return info;
	}

	bool setFunction(FunctionInfo& info, std::string_view view)
	{
		auto f = view.begin();
		const auto l = view.end();
		type::ast::FunctionType func;
		if (!boost::spirit::x3::phrase_parse(f, l, type::parser::function, boost::spirit::x3::ascii::space, func) || f != l)
			return false;

		info = createFunction(func);
		return true;
	}

	bool setType(TypeInfo& info, std::string_view view)
	{
		auto f = view.begin();
		const auto l = view.end();
		type::ast::ParsedType type;
		if (!boost::spirit::x3::phrase_parse(f, l, type::parser::parsedType, boost::spirit::x3::ascii::space, type) || f != l)
			return false;

		if (type.get().type() == typeid(type::ast::NamedType))
			info = createType(boost::get<type::ast::NamedType>(type));
		else
		{
			info.type = Type::function;
			info.function = createFunction(boost::get<type::ast::FunctionType>(type));
		}

		return true;
	}

	TEST_CASE("FunctionInfo from text")
	{
		auto info = FunctionInfo{"number function(string text)"};
		CHECK(info.isMethod == false);
		CHECK_FALSE(info.getResultTypeFunc);
		REQUIRE(info.parameters.size() == 1);
		CHECK(info.parameters[0].name() == "text");
		CHECK(info.parameters[0].type().type == Type::string);
		REQUIRE(info.results.size() == 1);
		CHECK(info.results[0].type == Type::number);

		info = FunctionInfo{"number, Player function(Player[] playerList)"};
		CHECK(info.isMethod == false);
		CHECK_FALSE(info.getResultTypeFunc);
		REQUIRE(info.parameters.size() == 1);
		CHECK(info.parameters[0].name() == "playerList");
		CHECK(info.parameters[0].type().type == Type::array);
		CHECK(info.parameters[0].type().name == "Player");
		REQUIRE(info.results.size() == 2);
		CHECK(info.results[0].type == Type::number);
		CHECK(info.results[1].type == Type::userdata);
		CHECK(info.results[1].name == "Player");

		auto dummyFunc = [](const an::Scope& scope, const ast::Arguments& args, const an::TypeInfo&) {
			return TypeInfo(Type::number);
		};

		info = FunctionInfo{"function()", dummyFunc};
		CHECK(info.getResultTypeFunc);
	}

	TEST_CASE("TypeInfo from text")
	{
		TypeInfo info = "number";
		CHECK(info.type == Type::number);
		CHECK(info.name.empty());

		info = TypeInfo{"string"};
		CHECK(info.type == Type::string);
		CHECK(info.name.empty());

		info = TypeInfo{"string[]"};
		CHECK(info.type == Type::array);
		CHECK(info.name == "string");

		info = TypeInfo{"Player"};
		CHECK(info.type == Type::userdata);
		CHECK(info.name == "Player");

		info = TypeInfo{"Player[]"};
		CHECK(info.type == Type::array);
		CHECK(info.name == "Player");

		info = TypeInfo{"function()"};
		CHECK(info.type == Type::function);
		CHECK(info.name.empty());
		CHECK(info.function.isMethod == false);
		CHECK(info.function.parameters.empty());
		CHECK(info.function.results.empty());

		info = TypeInfo{"number function(string text)"};
		CHECK(info.type == Type::function);
		CHECK(info.name.empty());
		CHECK(info.function.isMethod == false);
		REQUIRE(info.function.parameters.size() == 1);
		CHECK(info.function.parameters[0].name() == "text");
		CHECK(info.function.parameters[0].type().type == Type::string);
		REQUIRE(info.function.results.size() == 1);
		CHECK(info.function.results[0].type == Type::number);

		info = TypeInfo{"number, Player function(Player[] playerList)"};
		CHECK(info.type == Type::function);
		CHECK(info.name.empty());
		CHECK(info.function.isMethod == false);
		REQUIRE(info.function.parameters.size() == 1);
		CHECK(info.function.parameters[0].name() == "playerList");
		CHECK(info.function.parameters[0].type().type == Type::array);
		CHECK(info.function.parameters[0].type().name == "Player");
		REQUIRE(info.function.results.size() == 2);
		CHECK(info.function.results[0].type == Type::number);
		CHECK(info.function.results[1].type == Type::userdata);
		CHECK(info.function.results[1].name == "Player");

		auto dummyFunc = [](const an::Scope& scope, const ast::Arguments& args, const an::TypeInfo&) {
			return TypeInfo(Type::number);
		};

		info = TypeInfo{"number", dummyFunc};
		CHECK(info.type == Type::number);
		CHECK_FALSE(info.function.getResultTypeFunc);

		info = TypeInfo{"function()", dummyFunc};
		CHECK(info.type == Type::function);
		CHECK(info.function.getResultTypeFunc);
	}
} // namespace lac::an
