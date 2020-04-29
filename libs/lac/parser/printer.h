#pragma once

#include <lac/parser/ast.h>
#include <lac/core_api.h>

#include <nlohmann/json.hpp>

namespace lac
{
	CORE_API std::string toJson(const ast::Expression& ex);
	CORE_API std::string toJson(const ast::Block& block);
}
