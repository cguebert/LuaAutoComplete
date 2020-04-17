#pragma once

#include <lac/parser/ast.h>
#include <lac/analysis/type_info.h>

namespace lac::an
{
	class Scope;
}

namespace lac::helper
{
	CORE_API boost::optional<const ast::Numeral&> getNumeral(const ast::Arguments& args, size_t index = 0);
	CORE_API boost::optional<const std::string&> getLiteralString(const ast::Arguments& args, size_t index = 0);
	CORE_API an::TypeInfo getType(const an::Scope& scope, const ast::Arguments& args, size_t index = 0);
} // namespace lac::helper
