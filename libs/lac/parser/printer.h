#pragma once

#include <lac/parser/ast.h>

#include <nlohmann/json.hpp>

namespace lac
{
	nlohmann::json toJson(const ast::Expression& ex);
	nlohmann::json toJson(const ast::Block& block);
}
