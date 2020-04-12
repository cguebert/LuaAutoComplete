#pragma once

#include <lac/parser/ast.h>
#include <lac/core_api.h>

#include <nlohmann/json.hpp>

namespace lac
{
	CORE_API nlohmann::json toJson(const ast::Expression& ex);
	CORE_API nlohmann::json toJson(const ast::Block& block);
}
