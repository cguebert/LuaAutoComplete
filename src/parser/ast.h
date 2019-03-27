#pragma once

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/io.hpp>

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

		struct Operand
			: boost::spirit::x3::variant<
				  ExpressionConstant,
				  std::string,
				  double,
				  boost::spirit::x3::forward_ast<UnaryOperation>>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

		struct BinaryOperation;
		struct Expression
		{
			Operand first;
			std::list<BinaryOperation> rest;
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
	} // namespace ast
} // namespace lac
