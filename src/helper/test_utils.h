#pragma once

#include <parser/positions.h>

#include <boost/spirit/home/x3.hpp>
#include <doctest/doctest.h>
#include <string_view>

namespace lac
{
	namespace helper
	{
		template <class P, class... Args>
		bool test_parser(std::string_view input, const P& p, Args&... args)
		{
			auto f = input.begin();
			const auto l = input.end();
			pos::Positions positions{f, l};
			const auto parser = boost::spirit::x3::with<pos::position_tag>(std::ref(positions))[p];
			return boost::spirit::x3::parse(f, l, parser, args...) && f == l;
		}

		template <class P, class... Args>
		bool test_phrase_parser(std::string_view input, const P& p, Args&... args)
		{
			auto f = input.begin();
			const auto l = input.end();
			pos::Positions positions{f, l};
			const auto parser = boost::spirit::x3::with<pos::position_tag>(std::ref(positions))[p];
			return boost::spirit::x3::phrase_parse(f, l, parser, boost::spirit::x3::ascii::space, args...) && f == l;
		}

		template <class P, class V>
		void test_value(std::string_view input, const P& p, const V& value)
		{
			V v;
			CHECK(test_parser(input, p, v));
			CHECK(v == value);
		}

		template <class P>
		void test_value(std::string_view input, const P& p, const char* value)
		{
			std::string v;
			CHECK(test_parser(input, p, v));
			CHECK(v == std::string(value));
		}

#define TEST_VALUE(a, b, c) \
	SUBCASE(a) { test_value(a, b, c); }
	} // namespace helper
} // namespace lac

std::ostream& operator<<(std::ostream& os, const std::type_info& value);
