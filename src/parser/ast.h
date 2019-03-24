#pragma once

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
	} // namespace ast
} // namespace lac
