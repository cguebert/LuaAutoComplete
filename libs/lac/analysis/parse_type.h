#pragma once

#include <string_view>

namespace lac::an
{
	class FunctionInfo;
	class TypeInfo;

	bool setFunction(FunctionInfo& info, std::string_view view);
	bool setType(TypeInfo& info, std::string_view view);
} // namespace lac::an
