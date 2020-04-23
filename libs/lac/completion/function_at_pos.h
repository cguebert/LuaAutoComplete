#pragma once

#include <lac/analysis/type_info.h>
#include <lac/analysis/scope.h>

#include <boost/optional.hpp>

namespace lac::comp
{
	using ArgumentData = std::pair<an::TypeInfo, size_t>;
	
	// Find where we are in a function call (if we are)
	boost::optional<ArgumentData> getArgumentAtPos(const an::Scope& rootScope, std::string_view view, size_t pos = std::string_view::npos);
} // namespace lac::comp
