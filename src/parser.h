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

	boost::spirit::qi::rule<Iterator, std::string()> block, statement, returnStatement;
	boost::spirit::qi::symbols<char, std::string> keyword, binop, unop, fieldsep;
	boost::spirit::qi::rule<Iterator, std::string()> name;
	boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::qi::locals<char>> shortLiteralString;
	boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::qi::locals<std::string>> longLiteralString;
	boost::spirit::qi::rule<Iterator, std::string()> literalString;
	boost::spirit::qi::rule<Iterator, double()> numeral;

private:
	boost::spirit::qi::rule<Iterator, char(char)> escapedChar;
	boost::spirit::qi::rule<Iterator, std::string()> openLongBracket;
	boost::spirit::qi::rule<Iterator, void(std::string)> closeLongBacket;
};
