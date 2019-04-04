#pragma once

#include <parser/ast.h>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>

namespace lac
{
	namespace parser
	{
		namespace x3 = boost::spirit::x3;
		//	struct chunk_t;
		using chunk_type = x3::rule<struct chunk, ast::Block>;
		BOOST_SPIRIT_DECLARE(chunk_type);
	} // namespace parser

	parser::chunk_type chunkRule();
} // namespace lac
