#pragma once

#include <parser/chunk.h>
#include <string_view>

namespace lac::parser
{
	namespace x3 = boost::spirit::x3;

	using iterator_type = std::string_view::const_iterator;
	using positions_type = pos::Positions<iterator_type>;

	using pos_context_type = x3::context<pos::position_tag,
										 std::reference_wrapper<positions_type>>;

	using no_skip_context_type = x3::phrase_parse_context<x3::ascii::space_type>::type;
	using no_skip_pos_context_type = x3::context<pos::position_tag,
												 std::reference_wrapper<positions_type>,
												 no_skip_context_type>;

	using skipper_context_type = x3::phrase_parse_context<skipper_type>::type;
	using skipper_pos_context_type = x3::context<pos::position_tag,
												 std::reference_wrapper<positions_type>,
												 skipper_context_type>;

	// I had to decrypt the template error message to find this context type
	using chunk_pos_context_type = x3::context<pos::position_tag,
											   std::reference_wrapper<positions_type>,
											   x3::context<x3::skipper_tag,
														   const x3::with_directive<skipper_type,
																					pos::position_tag,
																					std::reference_wrapper<positions_type>>>>;
} // namespace lac::parser
