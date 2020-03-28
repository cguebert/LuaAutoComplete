#pragma once

#include <analysis/type_info.h>

namespace lac::comp
{
	an::TypeInfo getTypeAtPos(std::string_view view, size_t pos);
} // namespace lac::comp
