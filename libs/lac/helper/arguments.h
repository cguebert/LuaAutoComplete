#pragma once

#include <lac/parser/ast.h>

namespace lac::helper
{
	CORE_API boost::optional<const ast::Numeral&> getNumeral(const ast::Arguments& args, size_t index = 0);
	CORE_API boost::optional<const std::string&> getLiteralString(const ast::Arguments& args, size_t index = 0);
}
