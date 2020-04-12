#pragma once

#include <lac/parser/ast.h>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(lac::ast::LiteralString, value);

BOOST_FUSION_ADAPT_STRUCT(lac::ast::UnaryOperation, operation, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::BinaryOperation, operation, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::Expression, operand, binaryOperation);

BOOST_FUSION_ADAPT_STRUCT(lac::ast::FieldByExpression, key, value);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FieldByAssignment, name, value);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::TableConstructor, fields);

BOOST_FUSION_ADAPT_STRUCT(lac::ast::BracketedExpression, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::TableIndexExpression, expression);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::TableIndexName, name);

BOOST_FUSION_ADAPT_STRUCT(lac::ast::ParametersList, parameters, varargs);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionCallEnd, member, arguments);

BOOST_FUSION_ADAPT_STRUCT(lac::ast::PrefixExpression, start, rest);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::VariableFunctionCall, functionCall, postVariable);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::Variable, start, rest);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::VariableOrFunction, variable, functionCall, member);

BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionNameMember, name);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionName, start, rest, member);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionCallPostfix, tableIndex, functionCall);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionCall, start, rest);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionBody, parameters, block);

BOOST_FUSION_ADAPT_STRUCT(lac::ast::AssignmentStatement, variables, expressions);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::LocalAssignmentStatement, variables, expressions);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::LabelStatement, name);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::GotoStatement, label);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::DoStatement, block);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::WhileStatement, condition, block);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::RepeatStatement, block, condition);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::IfStatement, condition, block);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::IfThenElseStatement, first, rest, elseBlock);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::NumericalForStatement, variable, first, last, step, block);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::GenericForStatement, variables, expressions, block);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::FunctionDeclarationStatement, name, body);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::LocalFunctionDeclarationStatement, name, body);
BOOST_FUSION_ADAPT_STRUCT(lac::ast::ReturnStatement, expressions);

BOOST_FUSION_ADAPT_STRUCT(lac::ast::Block, statements, returnStatement);
