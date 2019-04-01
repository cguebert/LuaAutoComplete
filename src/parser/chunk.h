#pragma once

#include <parser/ast.h>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>


namespace lac
{

    namespace parser
    {
        namespace x3 = boost::spirit::x3;
        using chunk_type = x3::rule<struct chunk, ast::Block>;
        BOOST_SPIRIT_DECLARE(chunk_type);
    }

    parser::chunk_type chunkRule();
}
