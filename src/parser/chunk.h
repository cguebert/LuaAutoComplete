#pragma once

#include <parser/ast.h>
#include <parser/positions.h>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>

namespace lac
{
	namespace parser
	{
		namespace x3 = boost::spirit::x3;
		using chunk_type = x3::rule<struct chunk, ast::Block>;
		BOOST_SPIRIT_DECLARE(chunk_type);

		using variable_type = x3::rule<struct variable, ast::Variable>;
		BOOST_SPIRIT_DECLARE(variable_type);
	} // namespace parser

	parser::chunk_type chunkRule();
	parser::variable_type variableRule();

	// These skip comments and spaces
	bool parseString(std::string_view view, pos::Positions<std::string_view::const_iterator>& positions, ast::Block& block);
	bool parseString(std::string_view view, ast::Block& block);

	bool parseString(std::string_view view, ast::Variable& variable);
} // namespace lac
