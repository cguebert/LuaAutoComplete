#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4521) // multiple copy constructors specified
#endif

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/optional.hpp>

#include <iostream>
#include <string>

namespace lac
{
	namespace ast
	{
		enum class Operation
		{
			add,    // Addition (+)
			sub,    // Substraction (-)
			mul,    // Multiplication (*)
			div,    // Division (/)
			idiv,   // Floor division (//)
			mod,    // Modulo (%)
			pow,    // Exponentiation (^)
			unm,    // Unary negation (-)
			band,   // Bitwise AND (&)
			bor,    // Bitwise OR (|)
			bxor,   // Bitwise exclusive OR (~)
			bnot,   // Bitwise NOT (~)
			shl,    // Bitwise left shift (<<)
			shr,    // Bitwise right shift (>>)
			concat, // Concatenation (..)
			len,    // Length (#)
			lt,     // Less than (<)
			le,     // Less equal (<=)
			gt,     // Greater than (>)
			ge,     // Greater equal (>=)
			eq,     // Equal (==)
			ineq    // Inequal (~=)
		};

		enum class ExpressionConstant
		{
			nil,
			dots,
			False,
			True
		};

		struct UnaryOperation;
		using f_UnaryOperation = boost::spirit::x3::forward_ast<UnaryOperation>;

		struct Field;
		using FieldsList = std::list<Field>;

		struct TableConstructor
		{
			boost::optional<FieldsList> fields;
		};

		struct PrefixExpression;
		using f_PrefixExpression = boost::spirit::x3::forward_ast<PrefixExpression>;

		struct Operand
			: boost::spirit::x3::variant<
				  ExpressionConstant,
				  double,
				  std::string,
				  f_UnaryOperation,
				  TableConstructor,
				  f_PrefixExpression>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

		using NamesList = std::vector<std::string>;

		struct BinaryOperation;
		using f_BinaryOperation = boost::spirit::x3::forward_ast<BinaryOperation>;

		struct Expression
		{
			Operand operand;
			boost::optional<f_BinaryOperation> binaryOperation;
		};

		using ExpressionsList = std::list<Expression>;

		struct UnaryOperation
		{
			Operation operation;
			Expression expression;
		};

		struct BinaryOperation
		{
			Operation operation;
			Expression expression;
		};

		struct FieldByExpression
		{
			Expression key, value;
		};

		struct FieldByAssignment
		{
			std::string name;
			Expression value;
		};

		struct Field
			: boost::spirit::x3::variant<
				  FieldByExpression,
				  FieldByAssignment,
				  Expression>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

		struct BracketedExpression
		{
			Expression expression;
		};

		struct TableIndexExpression
		{
			Expression expression;
		};

		struct TableIndexName
		{
			std::string name;
		};

		struct ParametersList
		{
			NamesList parameters;
			bool varargs = false;
		};

		struct EmptyArguments
		{
		};

		struct Arguments
			: boost::spirit::x3::variant<
				  EmptyArguments,
				  ExpressionsList,
				  TableConstructor,
				  std::string>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

		struct FunctionCallEnd
		{
			boost::optional<std::string> member;
			Arguments arguments;
		};

		struct PostPrefix : boost::spirit::x3::variant<
								TableIndexExpression,
								TableIndexName,
								FunctionCallEnd>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

		struct PrefixExpression
		{
			boost::spirit::x3::variant<BracketedExpression, std::string> start;
			std::list<PostPrefix> rest;
		};

		struct VariableFunctionCall;
		using f_VariableFunctionCall = boost::spirit::x3::forward_ast<VariableFunctionCall>;

		struct VariablePostfix : boost::spirit::x3::variant<
									 TableIndexExpression,
									 TableIndexName,
									 f_VariableFunctionCall>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

		struct VariableFunctionCall
		{
			FunctionCallEnd functionCall;
			VariablePostfix postVariable;
		};

		struct Variable
		{
			boost::spirit::x3::variant<BracketedExpression, std::string> start;
			std::list<VariablePostfix> rest;
		};

		using VariablesList = std::list<Variable>;

		struct FunctionNameMember
		{
			std::string name;
		};

		struct FunctionName
		{
			std::string start;
			std::vector<std::string> rest;
			boost::optional<FunctionNameMember> member;
		};

		struct FunctionCall
		{
			Variable variable;
			FunctionCallEnd functionCall;
		};

		struct Block
		{
			std::string tmp;
			//	std::list<Statement> statements;
			//	std::list<Expression> returnStatement;
		};

		struct FunctionBody
		{
			boost::optional<ParametersList> parameters;
			Block block;
		};

		struct EmptyStatement
		{
		};

		struct AssignmentStatement
		{
			VariablesList variables;
			ExpressionsList expressions;
		};

		struct LocalAssignmentStatement
		{
			NamesList variables;
			boost::optional<ExpressionsList> expressions;
		};

		struct LabelStatement
		{
			std::string name;
		};

		struct BreakStatement
		{
		};

		struct GotoStatement
		{
			std::string label;
		};

		struct DoStatement
		{
			Block block;
		};

		struct WhileStatement
		{
			Expression condition;
			Block block;
		};

		struct RepeatStatement
		{
			Block block;
			Expression condition;
		};

		struct IfStatement
		{
			Expression condition;
			Block block;
		};

		struct IfThenElseStatement
		{
			IfStatement first;
			std::list<IfStatement> rest;
			boost::optional<Block> elseBlock;
		};

		struct NumericalForStatement
		{
			std::string variable;
			Expression first, last;
			boost::optional<Expression> step;
			Block block;
		};

		struct GenericForStatement
		{
			NamesList variables;
			ExpressionsList expressions;
			Block block;
		};

		struct FunctionDeclarationStatement
		{
			FunctionName name;
			FunctionBody body;
		};

		struct LocalFunctionDeclarationStatement
		{
			std::string name;
			FunctionBody body;
		};

		struct Statement : boost::spirit::x3::variant<
							   EmptyStatement,
							   AssignmentStatement>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

	} // namespace ast
} // namespace lac

#ifdef _MSC_VER
#pragma warning(pop)
#endif
