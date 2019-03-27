#pragma once

#include <parser/ast.h>

#include <iostream>

namespace lac
{
	void print(const ast::Expression& ex, std::ostream& out = std::cout);
}
