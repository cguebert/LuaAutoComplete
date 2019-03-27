#pragma once

#include <boost/fusion/include/adapt_struct.hpp>
#include <parser/ast.h>

BOOST_FUSION_ADAPT_STRUCT(lac::ast::UnaryOperation, operation, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::BinaryOperation, operation, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::Expression, first, rest);
