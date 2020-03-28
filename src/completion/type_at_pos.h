#pragma once

#include <analysis/type_info.h>

namespace lac::an
{
	class Scope;
}

namespace lac::comp
{
	an::TypeInfo getTypeAtPos(std::string_view view, size_t pos);
	an::TypeInfo getTypeAtPos(const an::Scope& scope, std::string_view view, size_t pos);
} // namespace lac::comp
