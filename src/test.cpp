#define BOOST_SPIRIT_DEBUG
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>
#include <boost/phoenix.hpp>
#include <doctest/doctest.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

using PosIterator = boost::spirit::line_pos_iterator<std::string_view::const_iterator>;

struct Annotation
{
	using result_type = void;
	template <class It>
	void operator()(std::string_view name, It f, It l) const
	{
		std::cerr << "+ " << name << " '" << std::string(f, l) << "'\n";
	}
};

struct ErrorHandler
{
	using result_type = qi::error_handler_result;
	template <class T1, class T2, class T3, class T4>
	qi::error_handler_result operator()(T1 b, T2 e, T3 where, T4 const& what) const
	{
		std::cerr << "Error: expecting " << what << " in line " << get_line(where) << ": \n"
				  << std::string(b, e) << "\n"
				  << std::setw(std::distance(b, where)) << '^' << "---- here\n";
		return qi::fail;
	}
};

template <class Iterator = PosIterator, class Skipper = ascii::space_type>
class TestParser
	: public qi::grammar<Iterator, std::string(), Skipper>
{
public:
	using iterator_type = Iterator;
	using skipper_type = Skipper;

	boost::spirit::qi::symbols<char, std::string> binaryOperator, unaryOperator, fieldSeparator;
	boost::spirit::qi::rule<Iterator, double()> numeral;
	boost::spirit::qi::rule<Iterator, char(char)> escapedChar;
	boost::spirit::qi::rule<Iterator, std::string(), Skipper> name, comment, numeralAsString,
		expression, expressionsList, prefixExpression, postPrefix,
		functionCallEnd, arguments, functionBody, functionDefinition, parametersList,
		tableConstructor, fieldsList, field,
		returnStatement, statement, block,
		variable, variablesList, namesList,
		simpleExpression, binaryOperation, unaryOperation, fieldSeparation,
		functionCall, fcPrefix, fcPostPrefix;
	boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::qi::locals<char>> literalString;

	phoenix::function<Annotation> annotate;
	phoenix::function<ErrorHandler> handler;

	TestParser(bool showDebug = false)
		: TestParser::base_type(expression, "expression")
	{
		using ascii::char_;
		using ascii::string;
		using namespace qi;

		// Names
		name %= eps[_val = ""]
				>> (alpha | char_('_'))
				>> *(alnum | char_('_'));

		namesList %= name >> *(',' >> name);

		// Short literal strings
		escapedChar = '\\' >> char_(_r1);
		literalString %= eps[_val = ""]
						 >> omit[char_("'\"")[_a = _1]]
						 >> *(escapedChar(_a) | (char_ - char_(_a)))
						 >> lit(_a);

		// Numerals
		numeral %= (lit("0x") >> hex)
				   | (lit("0X") >> hex)
				   | double_;
		numeralAsString = raw[numeral];

		// Comments
		comment %= "--" >> *(char_ - eol) >> -eol;

		// Table fields separators
		fieldSeparator.add(",", ",");
		fieldSeparator.add(";", ";");
		fieldSeparation %= fieldSeparator;

		using StringList = std::vector<std::string>;
		// Binary operations
		const StringList binops = {"+", "-", "*", "/", "//", "^", "%",
								   "&", "~", "|", ">>", "<<", "..",
								   "<", "<=", ">", ">=", "==", "~=",
								   "and", "or"};
		for (const auto& k : binops)
			binaryOperator.add(k, k);
		binaryOperation %= binaryOperator;

		// Unary operations
		const StringList unops = {"-", "not", "#", "~"};
		for (const auto& k : unops)
			unaryOperator.add(k, k);
		unaryOperation %= unaryOperator;

		// Complete syntax of Lua
		field = ('[' >> expression >> lit(']') >> lit('=') >> expression)
				| (name >> '=' >> expression)
				| expression;

		fieldsList %= field >> *(fieldSeparation >> field) >> -fieldSeparation;

		tableConstructor %= '{' >> -fieldsList >> '}';

		arguments %= ('(' >> -expressionsList >> ')')
					 | tableConstructor
					 | literalString;

		functionCallEnd %= -(':' >> name) >> arguments;

		functionBody %= '(' >> -parametersList >> ')' >> block >> lit("end");

		functionDefinition %= lit("function") >> functionBody;

		parametersList %= (namesList >> -(lit(',') >> lit("...")))
						  | lit("...");

		prefixExpression %= (('(' >> expression >> ')')
							 | name)
							>> *postPrefix;

		postPrefix %= ('[' >> expression >> ']')
					  | ('.' >> name)
					  | functionCallEnd;

		fcPrefix %= (('(' >> expression >> ')')
					 | name)
					>> *(fcPostPrefix >> !functionCallEnd);

		fcPostPrefix %= ('[' >> expression >> ']')
						| ('.' >> name)
						| functionCallEnd;
		functionCall %= fcPrefix >> functionCallEnd;

		simpleExpression %= lit("nil")
							| lit("false")
							| lit("true")
							| numeralAsString
							| literalString
							| lit("...")
							| functionDefinition
							| (unaryOperation >> expression)
							| tableConstructor
							| prefixExpression;
		expression %= simpleExpression >> -(binaryOperation >> expression);

		expressionsList %= expression >> *(',' >> expression);

		variable %= name;

		variablesList %= variable >> *(',' >> variable);

		returnStatement %= "return" >> -expressionsList >> -lit(';');

		statement %= ';'
					 | (variablesList >> '=' >> expressionsList)
					 | functionCall;

		block %= *statement >> -returnStatement;

#define ANNOTATE(name) \
	BOOST_SPIRIT_DEBUG_NODE(name);
		//	on_success(name, annotate(#name, _1, _3));
		//	on_error<fail>(name, handler(_1, _2, _3, _4));

		if (showDebug)
		{
			ANNOTATE(arguments);
			ANNOTATE(binaryOperation);
			ANNOTATE(block);
			ANNOTATE(comment);
			ANNOTATE(expression);
			ANNOTATE(expressionsList);
			ANNOTATE(field);
			ANNOTATE(fieldSeparation);
			ANNOTATE(fieldsList);
			ANNOTATE(functionBody);
			ANNOTATE(functionCall);
			ANNOTATE(functionDefinition);
			ANNOTATE(name);
			ANNOTATE(namesList);
			ANNOTATE(numeralAsString);
			ANNOTATE(parametersList);
			ANNOTATE(postPrefix);
			ANNOTATE(prefixExpression);
			ANNOTATE(returnStatement);
			ANNOTATE(simpleExpression);
			ANNOTATE(statement);
			ANNOTATE(tableConstructor);
			ANNOTATE(unaryOperation);
			ANNOTATE(variable);
			ANNOTATE(variablesList);
		}

#undef ANNOTATE
	}
};

template <class P>
bool test_phrase_parser(std::string_view input, const P& p)
{
	const PosIterator f(input.begin()), l(input.end());
	PosIterator i = f;

	if (!qi::phrase_parse(i, l, p, ascii::space))
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

bool test_phrase_parser(const std::string_view input, bool showDebug = false)
{
	TestParser<PosIterator, ascii::space_type> parser(showDebug);

	return test_phrase_parser(input, parser);
}

TEST_CASE("test")
{
	CHECK(test_phrase_parser("func(a, b)[c]:test('hello').d:e"));

	TestParser<PosIterator, ascii::space_type> parser(true);
	CHECK(test_phrase_parser("func()", parser.functionCall));
}
