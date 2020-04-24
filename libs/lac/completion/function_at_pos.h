#pragma once

#include <lac/analysis/type_info.h>
#include <lac/analysis/scope.h>

#include <boost/optional.hpp>

namespace lac::comp
{
	struct ArgumentData
	{
		an::TypeInfo parent;
		an::TypeInfo function;
		size_t argumentIndex;
	};
	
	// Find where we are in a function call (if we are)
	boost::optional<ArgumentData> getArgumentAtPos(const an::Scope& rootScope, std::string_view view, size_t pos = std::string_view::npos);
} // namespace lac::comp
