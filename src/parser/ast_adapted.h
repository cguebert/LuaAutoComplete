#pragma once

#include <boost/fusion/include/adapt_struct.hpp>
#include <parser/ast.h>

BOOST_FUSION_ADAPT_STRUCT(lac::ast::UnaryOperation, operation, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::BinaryOperation, operation, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::Expression, first, rest);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::ParametersList, parameters, varargs);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FieldByExpression, key, value);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FieldByAssignement, name, value);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::Block, tmp);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionBody, parameters, block);
