#define BOOST_SPIRIT_DEBUG
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/support/iterators/line_pos_iterator.hpp>
#include <doctest/doctest.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

namespace x3 = boost::spirit::x3;
namespace ascii = x3::ascii;

namespace parser
{
	// clang-format off
	struct binaryOperator_ : x3::symbols<std::string>
	{
		binaryOperator_()
		{
			add
				("+", "+")
				("-", "-")
				("*", "*")
				("/", "/")
				("//", "//")
				("^", "^")
				("%", "%")
				("&", "&")
				("~", "~")
				("|", "|")
				(">>", ">>")
				("<<", "<<")
				("..", "..")
				("<", "<")
				("<=", "<=")
				(">", ">")
				(">=", ">=")
				("==", "==")
				("~=", "~=")
				("and", "and")
				("or", "or")
				;
		}

	} binaryOperator;

	struct unaryOperator_ : x3::symbols<std::string>
	{
		unaryOperator_()
		{
			add
				("-", "-")
				("not", "not")
				("#", "#")
				("~", "~")
				;
		}

	} unaryOperator;

	struct fieldSeparator_ : x3::symbols<std::string>
	{
		fieldSeparator_()
		{
			add
				(",", ",")
				(";", ";")
				;
		}

	} fieldSeparator;
	// clang-format on

	const x3::rule<class numeral, double> numeral = "numeral";

	const x3::rule<class arguments, std::string> arguments = "arguments";
	const x3::rule<class binaryOperation, std::string> binaryOperation = "binaryOperation";
	const x3::rule<class block, std::string> block = "block";
	const x3::rule<class comment, std::string> comment = "comment";
	const x3::rule<class expression, std::string> expression = "expression";
	const x3::rule<class expressionsList, std::string> expressionsList = "expressionsList";
	const x3::rule<class field, std::string> field = "field";
	const x3::rule<class fieldSeparation, std::string> fieldSeparation = "fieldSeparation";
	const x3::rule<class fieldsList, std::string> fieldsList = "fieldsList";
	const x3::rule<class functionBody, std::string> functionBody = "functionBody";
	const x3::rule<class functionCall, std::string> functionCall = "functionCall";
	const x3::rule<class functionCallEnd, std::string> functionCallEnd = "functionCallEnd";
	const x3::rule<class functionDefinition, std::string> functionDefinition = "functionDefinition";
	const x3::rule<class literalString, std::string> literalString = "literalString";
	const x3::rule<class name, std::string> name = "name";
	const x3::rule<class namesList, std::string> namesList = "namesList";
	const x3::rule<class numeralAsString, std::string> numeralAsString = "numeralAsString";
	const x3::rule<class parametersList, std::string> parametersList = "parametersList";
	const x3::rule<class postPrefix, std::string> postPrefix = "postPrefix";
	const x3::rule<class prefixExpression, std::string> prefixExpression = "prefixExpression";
	const x3::rule<class returnStatement, std::string> returnStatement = "returnStatement";
	const x3::rule<class simpleExpression, std::string> simpleExpression = "simpleExpression";
	const x3::rule<class statement, std::string> statement = "statement";
	const x3::rule<class tableConstructor, std::string> tableConstructor = "tableConstructor";
	const x3::rule<class unaryOperation, std::string> unaryOperation = "unaryOperation";
	const x3::rule<class variable, std::string> variable = "variable";
	const x3::rule<class variablePostfix, std::string> variablePostfix = "variablePostfix";
	const x3::rule<class variablesList, std::string> variablesList = "variablesList";

	using ascii::char_;
	using ascii::string;
	using x3::_attr;
	using x3::alnum;
	using x3::alpha;
	using x3::double_;
	using x3::eol;
	using x3::eps;
	using x3::hex;
	using x3::lit;
	using x3::omit;
	using x3::raw;

	// Names
	const auto name_def = (alpha | char_('_'))
						  >> *(alnum | char_('_'));

	const auto namesList_def = name >> *(',' >> name);

	// Short literal strings
	const auto literalString_def = (lit('"') >> *("\\\"" | (char_ - '"')) >> '"')
								   | (lit('\'') >> *("\\'" | (char_ - '\'')) >> '\'');

	// Numerals0
	const auto numeral_def = (lit("0x") >> hex)
							 | (lit("0X") >> hex)
							 | double_;
	const auto numeralAsString_def = raw[numeral];

	// Comments
	const auto comment_def = "--" >> *(char_ - eol) >> -eol;

	// Table fields separators
	const auto fieldSeparation_def = fieldSeparator;

	using StringList = std::vector<std::string>;
	// Binary operations
	const auto binaryOperation_def = binaryOperator;

	// Unary operations
	const auto unaryOperation_def = unaryOperator;

	// Complete syntax of Lua
	const auto field_def = ('[' >> expression >> lit(']') >> lit('=') >> expression)
						   | (name >> '=' >> expression)
						   | expression;

	const auto fieldsList_def = field >> *(fieldSeparation >> field) >> -fieldSeparation;

	const auto tableConstructor_def = '{' >> -fieldsList >> '}';

	const auto arguments_def = ('(' >> -expressionsList >> ')')
							   | tableConstructor
							   | literalString;

	const auto functionCallEnd_def = -(':' >> name) >> arguments;

	const auto functionBody_def = '(' >> -parametersList >> ')' >> block >> lit("end");

	const auto functionDefinition_def = lit("function") >> functionBody;

	const auto parametersList_def = (namesList >> -(lit(',') >> lit("...")))
									| lit("...");

	const auto prefixExpression_def = (('(' >> expression >> ')')
									   | name)
									  >> *postPrefix;

	const auto postPrefix_def = ('[' >> expression >> ']')
								| ('.' >> name)
								| functionCallEnd;

	const auto variable_def = (('(' >> expression >> ')')
							   | name)
							  >> *variablePostfix;

	const auto variablePostfix_def = ('[' >> expression >> ']')
									 | ('.' >> name)
									 | (functionCallEnd >> variablePostfix);

	const auto functionCall_def = variable >> functionCallEnd;

	const auto simpleExpression_def = lit("nil")
									  | lit("false")
									  | lit("true")
									  | numeralAsString
									  | literalString
									  | lit("...")
									  | functionDefinition
									  | (unaryOperation >> expression)
									  | tableConstructor
									  | prefixExpression;
	const auto expression_def = simpleExpression >> -(binaryOperation >> expression);

	const auto expressionsList_def = expression >> *(',' >> expression);

	const auto variablesList_def = variable >> *(',' >> variable);

	const auto returnStatement_def = "return" >> -expressionsList >> -lit(';');

	const auto statement_def = ';'
							   | (variablesList >> '=' >> expressionsList)
							   | functionCall;

	const auto block_def = *statement >> -returnStatement;

	BOOST_SPIRIT_DEFINE(arguments);
	BOOST_SPIRIT_DEFINE(binaryOperation);
	BOOST_SPIRIT_DEFINE(block);
	BOOST_SPIRIT_DEFINE(comment);
	BOOST_SPIRIT_DEFINE(expression);
	BOOST_SPIRIT_DEFINE(expressionsList);
	BOOST_SPIRIT_DEFINE(field);
	BOOST_SPIRIT_DEFINE(fieldSeparation);
	BOOST_SPIRIT_DEFINE(fieldsList);
	BOOST_SPIRIT_DEFINE(functionBody);
	BOOST_SPIRIT_DEFINE(functionCall);
	BOOST_SPIRIT_DEFINE(functionCallEnd);
	BOOST_SPIRIT_DEFINE(functionDefinition);
	BOOST_SPIRIT_DEFINE(literalString);
	BOOST_SPIRIT_DEFINE(name);
	BOOST_SPIRIT_DEFINE(namesList);
	BOOST_SPIRIT_DEFINE(numeral);
	BOOST_SPIRIT_DEFINE(numeralAsString);
	BOOST_SPIRIT_DEFINE(parametersList);
	BOOST_SPIRIT_DEFINE(postPrefix);
	BOOST_SPIRIT_DEFINE(prefixExpression);
	BOOST_SPIRIT_DEFINE(returnStatement);
	BOOST_SPIRIT_DEFINE(simpleExpression);
	BOOST_SPIRIT_DEFINE(statement);
	BOOST_SPIRIT_DEFINE(tableConstructor);
	BOOST_SPIRIT_DEFINE(unaryOperation);
	BOOST_SPIRIT_DEFINE(variable);
	BOOST_SPIRIT_DEFINE(variablePostfix);
	BOOST_SPIRIT_DEFINE(variablesList);
	/*
	escapedChar = '\\' >> char_(_r1);
	shortLiteralString %= eps[_val = ""]
		>> omit[char_("'\"")[_a = _1]]
		>> *(escapedChar(_a) | (char_ - char_(_a)))
		>> lit(_a);
*/
	struct _string_quote
	{
	};

	auto set_quote = [](auto& ctx) { x3::get<_string_quote>(ctx) = x3::_attr(ctx); };
	auto is_quote = [](auto& ctx) { x3::_pass(ctx) = (x3::get<_string_quote>(ctx) == x3::_attr(ctx)); };

	const x3::rule<class testString, std::string> testString = "testString";

	const auto testString_def = omit[char_("'\"")[set_quote]]
							>> *(('\\' >> char_[is_quote])
								 | (char_ - char_[is_quote]))
							>> char_[is_quote];

	BOOST_SPIRIT_DEFINE(testString);

} // namespace parser

using PosIterator = boost::spirit::line_pos_iterator<std::string_view::const_iterator>;

template <class P>
bool test_phrase_parser(std::string_view input, const P& p)
{
	const PosIterator f(input.begin()), l(input.end());
	PosIterator i = f;

	if (!x3::phrase_parse(i, l, p, ascii::space))
	{
		std::cerr << "------------------------------------\n"
				  << "Could not parse input: " << input << '\n'
				  << "------------------------------------\n";
		return false;
	};

	if (i != l)
	{
		std::cerr << "------------------------------------\n"
				  << "Error while parsing:\n"
				  << input << '\n'
				  << std::setw(std::distance(f, i) + 1) << '^' << "---- here\n"
				  << "------------------------------------\n";
		return false;
	}
	return true;
}

TEST_CASE("test")
{
	//CHECK(test_phrase_parser("func(a, b)[c]:test('hello').d", parser::expression));

	const auto r = x3::with<parser::_string_quote>(' ')[parser::testString];

	CHECK(test_phrase_parser("'hello'", r));
	CHECK(test_phrase_parser("\"hello\"", r));
	CHECK(test_phrase_parser("func(a, b)[c]:test('hello').d", parser::variable));
	//	CHECK_FALSE(test_phrase_parser("func(a, b)[c]:test('hello')", parser::variable));
}
