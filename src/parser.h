#pragma once

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>

#include <string_view>
#include <string>

class Parser
{
public:
	using Iterator = std::string_view::iterator;
	Parser();

	boost::spirit::qi::symbols<char, std::string> keyword, binaryOperation, unaryOperation, fieldSeparator;
	boost::spirit::qi::rule<Iterator, std::string()> name, literalString, comment, shortComment;
	boost::spirit::qi::rule<Iterator, double()> numeral;
	boost::spirit::qi::rule<Iterator, std::string()> numeralAsString;

	boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::qi::rule<Iterator>> block,
		statement, returnStatement, label, functionName, variable, variablesList, namesList,
		expression, expressionsList, prefixExpression, functionCall, arguments, functionDefinition, functionBody,
		parametersList, tableConstructor, fieldsList, field;

	boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::qi::locals<char>> shortLiteralString;
	boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::qi::locals<std::string>> longLiteralString, longComment;

	boost::spirit::qi::rule<Iterator> skipper;

	class ChunkGrammar : public boost::spirit::qi::grammar<Iterator, boost::spirit::qi::rule<Iterator>>
	{
	public:
		ChunkGrammar(Parser& parser);

	private:
		boost::spirit::qi::rule<Iterator, boost::spirit::qi::rule<Iterator>> m_start;
		Parser& m_parser;
	};

private:
	boost::spirit::qi::rule<Iterator, char(char)> escapedChar;
	boost::spirit::qi::rule<Iterator, std::string()> openLongBracket;
	boost::spirit::qi::rule<Iterator, void(std::string)> closeLongBacket;
};
