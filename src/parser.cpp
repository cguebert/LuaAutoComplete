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

	// Literal strings
	escapedChar = '\\' >> char_(_r1);
	shortLiteralString %= eps[_val = ""]
						  >> omit[char_("'\"")[_a = _1]]
						  >> *(escapedChar(_a) | (char_ - char_(_a)))
						  >> lit(_a);

	// Table fields separators
	fieldsep.add(",", ",");
	fieldsep.add(";", ";");

	// Binary operations
	const StringList binops = {"+", "-", "*", "/", "//", "^", "%",
							   "&", "~", "|", ">>", "<<", "..",
							   "<", "<=", ">", ">=", "==", "~=",
							   "and", "or"};
	for (const auto& k : binops)
		binop.add(k, k);

	// Unary operations
	const StringList unops = {"-", "not", "#", "~"};
	for (const auto& k : unops)
		unop.add(k, k);
}
