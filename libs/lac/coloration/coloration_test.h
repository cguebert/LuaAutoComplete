#pragma once

#include <lac/parser/positions.h>
#include <lac/lac_api.h>

#include <string_view>

namespace lac
{
	LAC_API void printProgram(std::string_view text, const lac::pos::Elements& elements);
}
