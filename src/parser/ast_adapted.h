#pragma once

#include <boost/fusion/include/adapt_struct.hpp>
#include <parser/ast.h>

BOOST_FUSION_ADAPT_STRUCT(lac::ast::UnaryOperation, operation, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::BinaryOperation, operation, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::Expression, operand, binaryOperation);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FieldByExpression, key, value);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FieldByAssignement, name, value);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::TableConstructor, fields);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::BracketedExpression, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::TableIndexExpression, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::TableIndexName, name);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::ParametersList, parameters, varargs);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionCallEnd, member, arguments);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::PrefixExpression, start, rest);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::VariableFunctionCall, functionCall, postVariable);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionNameMember, name);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionName, start, rest, member);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionCall, variable, functionCall);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::Variable, start, rest);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::Block, tmp);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionBody, parameters, block);
