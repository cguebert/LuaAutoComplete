#include <boost/config/warning_disable.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/support/iterators/line_pos_iterator.hpp>
#include <doctest/doctest.h>

#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <string_view>

namespace x3 = boost::spirit::x3;
namespace ascii = x3::ascii;

namespace test
{
	namespace ast
	{
		enum class Operation
		{
			add,    // Addition (+)
			sub,    // Substraction (-)
			mul,    // Multiplication (*)
			div,    // Division (/)
			idiv,   // Floor division (//)
			mod,    // Modulo (%)
			pow,    // Exponentiation (^)
			unm,    // Unary negation (-)
			band,   // Bitwise AND (&)
			bor,    // Bitwise OR (|)
			bxor,   // Bitwise exclusive OR (~)
			bnot,   // Bitwise NOT (~)
			shl,    // Bitwise left shift (<<)
			shr,    // Bitwise right shift (>>)
			concat, // Concatenation (..)
			len,    // Length (#)
			lt,     // Less than (<)
			le,     // Less equal (<=)
			gt,     // Greater than (>)
			ge,     // Greater equal (>=)
			eq,     // Equal (==)
			ineq    // Inequal (~=)
		};

		enum class ExpressionConstant
		{
			nil,
			dots,
			False,
			True
		};

		struct UnaryOperation;

		struct Operand
			: x3::variant<
				  ExpressionConstant,
				  std::string,
				  double,
				  x3::forward_ast<UnaryOperation>>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

		struct BinaryOperation;
		struct Expression
		{
			Operand first;
			std::list<BinaryOperation> rest;
		};

		using ExpressionsList = std::list<Expression>;

		struct UnaryOperation
		{
			Operation operation;
			Expression expression;
		};

		struct BinaryOperation
		{
			Operation operation;
			Expression expression;
		};

	} // namespace ast
} // namespace test

BOOST_FUSION_ADAPT_STRUCT(test::ast::UnaryOperation, operation, expression);
BOOST_FUSION_ADAPT_STRUCT(test::ast::BinaryOperation, operation, expression);
BOOST_FUSION_ADAPT_STRUCT(test::ast::Expression, first, rest);

namespace test
{
	namespace x3 = boost::spirit::x3;
	namespace ascii = x3::ascii;
	using ascii::char_;
	using ascii::string;
	using x3::_attr;
	using x3::_pass;
	using x3::alnum;
	using x3::alpha;
	using x3::double_;
	using x3::eol;
	using x3::eps;
	using x3::get;
	using x3::hex;
	using x3::lexeme;
	using x3::lit;
	using x3::omit;
	using x3::raw;
	using x3::with;

	// clang-format off
	struct keyword_ : x3::symbols<std::string>
	{
		keyword_()
		{
			add
				("and", "and")
				("break", "break")
				("do", "do")
				("else", "else")
				("elseif", "elseif")
				("end", "end")
				("false", "false")
				("for", "for")
				("function", "function")
				("goto", "goto")
				("if", "if")
				("in", "in")
				("local", "local")
				("nil", "nil")
				("not", "not")
				("or", "or")
				("repeat", "repeat")
				("return", "return")
				("then", "then")
				("true", "true")
				("until", "until")
				("while", "while");
		}
	} keyword;

	struct binaryOperator_ : x3::symbols<ast::Operation>
	{
		binaryOperator_()
		{
			using Op = ast::Operation;
			add
				("+",   Op::add)
				("-",   Op::sub)
				("*",   Op::mul)
				("/",   Op::div)
				("//",  Op::idiv)
				("%",   Op::mod)
				("^",   Op::pow)
				("&",   Op::band)
				("|",   Op::bor)
				("~",   Op::bxor)
				("<<",  Op::shl)
				(">>",  Op::shr)
				("..",  Op::concat)
				("<",   Op::lt)
				("<=",  Op::le)
				(">",   Op::gt)
				(">=",  Op::ge)
				("==",  Op::eq)
				("~=",  Op::ineq)
				("and", Op::band)
				("or",  Op::bor);
		}
	} binaryOperator;

	struct unaryOperator_ : x3::symbols<ast::Operation>
	{
		using Op = ast::Operation;
		unaryOperator_()
		{
			add
				("-",   Op::unm)
				("#",   Op::len)
				("~",   Op::bnot)
				("not", Op::bnot);
		}
	} unaryOperator;

	struct fieldSeparator_ : x3::symbols<std::string>
	{
		fieldSeparator_()
		{
			add
				(",", ",")
				(";", ";");
		}
	} fieldSeparator;

	struct expressionConstant_ : x3::symbols<ast::ExpressionConstant>
	{
		expressionConstant_()
		{
			using EC = ast::ExpressionConstant;
			add
				("nil", EC::nil)
				("false", EC::False)
				("true", EC::True)
				("...", EC::dots);
		}
	} expressionConstant;
	// clang-format on

	const x3::rule<class name, std::string> name = "name";
	const x3::rule<class namesList, std::string> namesList = "namesList";

	const x3::rule<class openLongBracket> openLongBracket = "openLongBracket";
	const x3::rule<class closeLongBacket> closeLongBacket = "closeLongBacket";
	const x3::rule<class longLiteralString, std::string> longLiteralString = "longLiteralString";
	const x3::rule<class literalString, std::string> literalString = "literalString";

	const x3::rule<class numeral, double> numeral = "numeral";
	const x3::rule<class numeralAsString, std::string> numeralAsString = "numeralAsString";

	const x3::rule<class shortComment, std::string> shortComment = "shortComment";
	const x3::rule<class longComment, std::string> longComment = "longComment";
	const x3::rule<class comment, std::string> comment = "comment";

	const x3::rule<class field, std::string> field = "field";
	const x3::rule<class fieldsList, std::string> fieldsList = "fieldsList";
	const x3::rule<class tableConstructor, std::string> tableConstructor = "tableConstructor";

	const x3::rule<class parametersList, std::string> parametersList = "parametersList";
	const x3::rule<class arguments, std::string> arguments = "arguments";
	const x3::rule<class functionBody, std::string> functionBody = "functionBody";
	const x3::rule<class functionCall, std::string> functionCall = "functionCall";
	const x3::rule<class functionCallEnd, std::string> functionCallEnd = "functionCallEnd";
	const x3::rule<class functionDefinition, std::string> functionDefinition = "functionDefinition";
	const x3::rule<class functionName, std::string> functionName = "functionName";

	const x3::rule<class prefixExpression, std::string> prefixExpression = "prefixExpression";
	const x3::rule<class postPrefix, std::string> postPrefix = "postPrefix";
	const x3::rule<class variable, std::string> variable = "variable";
	const x3::rule<class variablePostfix, std::string> variablePostfix = "variablePostfix";
	const x3::rule<class variablesList, std::string> variablesList = "variablesList";

	const x3::rule<class unaryOperation, ast::UnaryOperation> unaryOperation = "unaryOperation";
	const x3::rule<class binaryOperation, ast::BinaryOperation> binaryOperation = "binaryOperation";
	const x3::rule<class simpleExpression, ast::Operand> simpleExpression = "simpleExpression";
	const x3::rule<class expression, ast::Expression> expression = "expression";
	const x3::rule<class expressionsList, ast::ExpressionsList> expressionsList = "expressionsList";

	const x3::rule<class label, std::string> label = "label";
	const x3::rule<class statement, std::string> statement = "statement";
	const x3::rule<class returnStatement, std::string> returnStatement = "returnStatement";

	const x3::rule<class block, std::string> block = "block";
	const x3::rule<class chunk, std::string> chunk = "chunk";

	// Names
	const auto name_def = lexeme[((alpha | char_('_'))
								  >> *(alnum | char_('_')))
								 - keyword];

	const auto namesList_def = name % ',';

	// Short literal strings
	auto quotedString = [](char c) {
		return lit(c)
			   >> *(('\\' >> char_(c)) // Escaped quote
					| (char_ - c))     // Any character but the quote
			   >> c;
	};

	// Long literal strings
	struct long_bracket_tag
	{
	};
	auto set_bracket = [](auto& ctx) { get<long_bracket_tag>(ctx) = _attr(ctx); };
	auto is_bracket = [](auto& ctx) { _pass(ctx) = (x3::get<long_bracket_tag>(ctx) == _attr(ctx)); };
	const auto openLongBracket_def = '[' >> (*char_('='))[set_bracket] >> '[';
	const auto closeLongBacket_def = ']' >> (*char_('='))[is_bracket] >> ']';
	const auto longLiteralString_def = with<long_bracket_tag>(std::string())
		[omit[openLongBracket]
		 >> *(char_ - closeLongBacket)
		 >> omit[closeLongBacket]];

	// Literal strings
	const auto literalString_def = lexeme[quotedString('\'')
										  | quotedString('"')
										  | longLiteralString];

	// Numerals
	const auto numeral_def = (lit("0x") >> hex)
							 | (lit("0X") >> hex)
							 | double_;
	const auto numeralAsString_def = raw[numeral];

	// Comments
	const auto shortComment_def = "--" >> lexeme[*(char_ - eol)] >> -eol;
	const auto longComment_def = with<long_bracket_tag>(std::string())
		["--"
		 >> omit[openLongBracket]
		 >> lexeme[*(char_ - closeLongBacket)]
		 >> omit[closeLongBacket]];
	const auto comment_def = longComment | shortComment;

	// A skipper that ignore whitespace and comments
	const x3::rule<class skipper> skipper = "skipper";

	const auto skipper_def = ascii::space
							 | omit[shortComment]
							 | omit[longComment];

	//*** Complete syntax of Lua ***
	// Table and fields
	const auto field_def = ('[' >> expression >> lit(']') >> lit('=') >> expression)
						   | (name >> '=' >> expression)
						   | expression;

	const auto fieldsList_def = field >> *(fieldSeparator >> field) >> -fieldSeparator;

	const auto tableConstructor_def = '{' >> -fieldsList >> '}';

	// Functions
	const auto parametersList_def = (namesList >> -(lit(',') >> lit("...")))
									| lit("...");

	const auto arguments_def = ('(' >> -expressionsList >> ')')
							   | tableConstructor
							   | literalString;

	const auto functionBody_def = '(' >> -parametersList >> ')' >> block >> lit("end");

	const auto functionDefinition_def = lit("function") >> functionBody;

	const auto functionCall_def = variable >> functionCallEnd;

	const auto functionCallEnd_def = -(':' >> name) >> arguments;

	const auto functionName_def = name >> *('.' >> name) >> -(':' >> name);

	// Variables
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

	const auto variablesList_def = variable % ',';

	// Expressions
	const auto unaryOperation_def = unaryOperator >> expression;
	const auto simpleExpression_def = expressionConstant
									  | numeral
									  | literalString
									  | unaryOperation
		/*
									  | functionDefinition
									  | tableConstructor
									  | prefixExpression
		*/
		;
	const auto binaryOperation_def = binaryOperator >> expression;
	const auto expression_def = simpleExpression >> -binaryOperation;

	const auto expressionsList_def = expression >> *(',' >> expression);

	// Statements
	const auto label_def = "::" >> name >> "::";

	const auto returnStatement_def = "return" >> -expressionsList >> -lit(';');

	const auto statement_def = ';'
							   | (variablesList >> '=' >> expressionsList)
							   | functionCall
							   | label
							   | "break"
							   | ("goto" >> name)
							   | ("do" >> block >> "end")
							   | ("while" >> expression >> "do" >> block >> "end")
							   | ("repeat" >> block >> "until" >> expression)
							   | ("if" >> expression >> "then" >> block >> *("elseif" >> expression >> "then" >> block) >> -("else" >> block) >> "end")
							   | ("for" >> name >> '=' >> expression >> ',' >> expression >> -(',' >> expression) >> "do" >> block >> "end")
							   | ("for" >> namesList >> "in" >> expressionsList >> "do" >> block >> "end")
							   | ("function" >> functionName >> functionBody)
							   | ("local" >> lit("function") >> name >> functionBody)
							   | ("local" >> namesList >> -('=' >> expressionsList));

	// Blocks
	const auto block_def = *statement >> -returnStatement;
	const auto chunk_def = block;

	BOOST_SPIRIT_DEFINE(name, namesList,
						openLongBracket, closeLongBacket,
						longLiteralString, literalString,
						numeral, numeralAsString,
						shortComment, longComment, comment,
						skipper,
						field, fieldsList, tableConstructor,
						parametersList, arguments,
						functionBody, functionCall, functionCallEnd,
						functionDefinition, functionName,
						prefixExpression, postPrefix,
						variable, variablePostfix, variablesList,
						unaryOperation, binaryOperation,
						simpleExpression, expression, expressionsList,
						label, returnStatement, statement,
						block, chunk);

} // namespace test

using PosIterator = boost::spirit::line_pos_iterator<std::string_view::const_iterator>;

template <class P, class... Args>
bool test_phrase_parser(std::string_view input, const P& p, Args&... args)
{
	const PosIterator f(input.begin()), l(input.end());
	PosIterator i = f;

	if (!x3::phrase_parse(i, l, p, ascii::space, args...))
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

	using namespace test::ast;
	Operand op;
	CHECK(test_phrase_parser("nil", test::simpleExpression, op));
	CHECK(boost::get<ExpressionConstant>(op.get()) == ExpressionConstant::nil);
	CHECK(test_phrase_parser("true", test::simpleExpression, op));
	CHECK(boost::get<ExpressionConstant>(op.get()) == ExpressionConstant::True);
	CHECK(test_phrase_parser("...", test::simpleExpression, op));
	CHECK(boost::get<ExpressionConstant>(op.get()) == ExpressionConstant::dots);

	CHECK(test_phrase_parser("42", test::simpleExpression, op));
	CHECK(boost::get<double>(op.get()) == 42.0);
	CHECK(test_phrase_parser("'hello'", test::simpleExpression, op));
	CHECK(boost::get<std::string>(op.get()) == "hello");

	Operation v;
	CHECK(test_phrase_parser("#", test::unaryOperator, v));
	CHECK(v == Operation::len);

	UnaryOperation uo;
	CHECK(test_phrase_parser("-42", test::unaryOperation, uo));
	CHECK(uo.operation == Operation::unm);
	CHECK(boost::get<double>(uo.expression.first.get()) == 42.0);
	CHECK(test_phrase_parser("not true", test::unaryOperation, uo));
	CHECK(uo.operation == Operation::bnot);
	CHECK(boost::get<ExpressionConstant>(uo.expression.first.get()) == ExpressionConstant::True);
/*	CHECK(test_phrase_parser("#array", test::unaryOperation, uo));
	CHECK(uo.operation == Operation::len);
	CHECK(boost::get<std::string>(uo.expression.first.get()) == "array");*/

	//	CHECK(test_phrase_parser("func(a, b)[c]:test('hello').d", test::variable));
	//	CHECK_FALSE(test_phrase_parser("func(a, b)[c]:test('hello')", parser::variable));
}
