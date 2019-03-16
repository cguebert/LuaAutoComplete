#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

template <class Iterator>
class Parser
{
public:
	Parser()
	{
		using ascii::char_;
		using qi::_1;
		using qi::_val;
		using qi::eps;
		using qi::lexeme;
		using qi::lit;

		const std::vector<std::string> keywordsStr = {"and", "break", "do", "else", "elseif",
													  "end", "false", "for", "function", "goto", "if",
													  "in", "local", "nil", "not", "or", "repeat",
													  "return", "then", "true", "until", "while"};
		for (const auto& k : keywordsStr)
			keyword.add(k, k);

		name %= (eps[_val = ""]
				 >> (qi::alpha | qi::char_('_'))
				 >> *(qi::alnum | qi::char_('_')))
				- keyword;
	}

	qi::symbols<char, std::string> keyword;
	qi::rule<Iterator, std::string()> name;
};

bool parse_numbers(const std::string& str, std::vector<double>& v)
{
	using ascii::space;
	using qi::double_;

	auto addToVec = [&v](double value) { v.push_back(value); };

	auto first = str.begin();
	const auto last = str.end();
	bool r = qi::phrase_parse(first, last,
							  //  Begin grammar
							  (
								  double_[addToVec]
								  >> *(',' >> double_[addToVec])),
							  //  End grammar
							  space);

	if (first != last) // fail if we did not get a full match
		return false;
	return r;
}

template <class P, class... Args>
bool test_parser(std::string_view input, const P& p, Args&... args)
{
	using boost::spirit::qi::parse;

	auto f = input.begin();
	const auto l = input.end();
	return parse(f, l, p, args...) && f == l;
}

template <class P, class... Args>
bool test_phrase_parser(std::string_view input, const P& p, Args&... args)
{
	using boost::spirit::qi::phrase_parse;
	using boost::spirit::qi::ascii::space;

	auto f = input.begin();
	const auto l = input.end();
	return phrase_parse(f, l, p, space, args...) && f == l;
}

Parser<std::string_view::iterator> parser;

TEST_CASE("keywords")
{
	std::string v;
	CHECK(test_parser("and", parser.keyword, v));
	CHECK(v == "and");

	CHECK(test_parser("nil", parser.keyword, v));
	CHECK(v == "nil");

	CHECK_FALSE(test_parser("toto", parser.keyword));
}

TEST_CASE("names")
{
	std::string v;
	CHECK(test_parser("toto", parser.name, v));
	CHECK(v == "toto");

	CHECK(test_parser("_test", parser.name, v));
	CHECK(v == "_test");

	CHECK(test_parser("_123", parser.name, v));
	CHECK(v == "_123");

	CHECK(test_parser("_t1o2t3o4", parser.name, v));
	CHECK(v == "_t1o2t3o4");

	CHECK_FALSE(test_parser("123toto", parser.name));

	CHECK_FALSE(test_parser("break", parser.name));
	CHECK_FALSE(test_parser("while", parser.name));
}

int main(int argc, char** argv)
{
	doctest::Context context;
	context.applyCommandLine(argc, argv);

	int res = context.run();

	if (context.shouldExit() || res)
		return res;

	return 0;
}
