#pragma once

#include <lac/parser/ast.h>
#include <lac/lac_api.h>

#include <nlohmann/json.hpp>

namespace lac
{
	LAC_API nlohmann::json toJson(const ast::Expression& ex);
	LAC_API nlohmann::json toJson(const ast::Block& block);
}
