#pragma once

#include <parser/ast.h>
#include <parser/positions.h>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>

namespace lac::parser
{
	using skipper_type = boost::spirit::x3::rule<struct skipper>;
	BOOST_SPIRIT_DECLARE(skipper_type);

	using chunk_type = boost::spirit::x3::rule<struct chunk, ast::Block>;
	BOOST_SPIRIT_DECLARE(chunk_type);

	using variable_or_function_type = boost::spirit::x3::rule<struct variableOrFunction, ast::VariableOrFunction>;
	BOOST_SPIRIT_DECLARE(variable_or_function_type);

	skipper_type skipperRule();
	chunk_type chunkRule();
	variable_or_function_type variableOrFunctionRule();
} // namespace lac::parser
