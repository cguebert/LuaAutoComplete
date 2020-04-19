#pragma once

#include <string_view>

namespace lac::an
{
	class TypeInfo;

	bool setType(TypeInfo& info, std::string_view view);
} // namespace lac::an
