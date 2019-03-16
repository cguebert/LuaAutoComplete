#include "parser.h"

#include <boost/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

Parser::Parser()
{
	using ascii::char_;
	using ascii::string;
	using namespace qi;

	using StringList = std::vector<std::string>;

	// Language keywords
	const StringList keywords = {"and", "break", "do", "else", "elseif",
								 "end", "false", "for", "function", "goto", "if",
								 "in", "local", "nil", "not", "or", "repeat",
								 "return", "then", "true", "until", "while"};
	for (const auto& k : keywords)
		keyword.add(k, k);

	// Names
	name %= (eps[_val = ""]
			 >> (alpha | char_('_'))
			 >> *(alnum | char_('_')))
			- keyword;

	// Short literal strings
	escapedChar = '\\' >> char_(_r1);
	shortLiteralString %= eps[_val = ""]
						  >> omit[char_("'\"")[_a = _1]]
						  >> *(escapedChar(_a) | (char_ - char_(_a)))
						  >> lit(_a);

	// Long literal strings
	openLongBracket = '[' >> *char_('=') >> '[';
	closeLongBacket = ']' >> string(_r1) >> ']';
	longLiteralString %= eps[_val = ""]
						 >> omit[openLongBracket[_a = _1]]
						 >> *(char_ - closeLongBacket(_a))
						 >> closeLongBacket(_a);

	// Literal strings
	literalString = shortLiteralString | longLiteralString;

	// Numerals
	numeral %= (lit("0x") >> hex)
			   | (lit("0X") >> hex)
			   | double_;
	numeralAsString = raw[numeral];

	// Comments
	shortComment %= "--" >> *(char_ - eol) >> -eol;
	longComment %= "--"
				   >> omit[openLongBracket[_a = _1]]
				   >> *(char_ - closeLongBacket(_a))
				   >> closeLongBacket(_a);
	comment %= longComment | shortComment;

	// Table fields separators
	fieldSeparator.add(",", ",");
	fieldSeparator.add(";", ";");

	// Binary operations
	const StringList binops = {"+", "-", "*", "/", "//", "^", "%",
							   "&", "~", "|", ">>", "<<", "..",
							   "<", "<=", ">", ">=", "==", "~=",
							   "and", "or"};
	for (const auto& k : binops)
		binaryOperation.add(k, k);

	// Unary operations
	const StringList unops = {"-", "not", "#", "~"};
	for (const auto& k : unops)
		unaryOperation.add(k, k);

	// Complete syntax of Lua
	field = ('[' >> expression >> lit(']') >> lit('=') >> expression)
			| (name >> '=' >> expression)
			| expression;

	fieldsList %= field >> *(fieldSeparator >> field) >> -fieldSeparator;

	tableConstructor %= '{' >> -fieldsList >> '}';

	parametersList %= (namesList >> -(lit(',') >> lit("...")))
					  | lit("...");

	functionBody %= '(' >> -parametersList >> ')' >> block >> lit('end');

	functionDefinition %= lit("function") >> functionBody;

	arguments %= ('(' >> expressionsList >> ')')
				 | tableConstructor
				 | literalString;

	functionCall %= (prefixExpression >> arguments)
					| (prefixExpression >> ':' >> name >> arguments);

	prefixExpression %= variable
						| functionCall
						| ('(' >> expression >> ')');

	expression %= lit("nil")
				  | lit("false")
				  | lit("true")
				  | numeralAsString
				  | literalString
				  | lit("...")
				  | functionDefinition
				  | prefixExpression
				  | tableConstructor
				  | (expression >> binaryOperation >> expression)
				  | (unaryOperation >> expression);

	expressionsList %= expression >> *(',' >> expression);

	namesList %= name >> *(',' >> name);

	variable %= name
				| (prefixExpression >> '[' >> expression >> ']')
				| (prefixExpression >> '.' >> name);

	variablesList %= variable >> *(',' >> variable);

	functionName %= name >> *(',' >> name) >> -(':' >> name);

	label %= "::" >> name >> "::";

	returnStatement %= "return" >> -expressionsList >> -lit(';');

	statement %= ';'
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

	block %= *statement >> -returnStatement;
}
