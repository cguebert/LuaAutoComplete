#pragma once

#include <parser/positions.h>

#include <boost/spirit/home/x3.hpp>
#include <string_view>

namespace lac
{
	namespace parser
	{
		namespace x3 = boost::spirit::x3;

		using iterator_type = std::string_view::const_iterator;
		using phrase_context_type = x3::phrase_parse_context<x3::ascii::space_type>::type;
		using positions_type = pos::Positions<iterator_type>;

		using context_type = x3::context<pos::position_tag,
										 std::reference_wrapper<positions_type>,
										 phrase_context_type>;
	} // namespace parser
} // namespace lac
