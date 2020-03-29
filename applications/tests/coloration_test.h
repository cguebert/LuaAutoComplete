#pragma once

#include <lac/parser/positions.h>

#include <string_view>

namespace color
{
	void printProgram(std::string_view text, const lac::pos::Elements& elements);
}
