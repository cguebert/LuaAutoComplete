#include <lac/helper/arguments.h>
#include <lac/analysis/get_type.h>

namespace lac::helper
{
	boost::optional<const ast::Numeral&> getNumeral(const ast::Arguments& args, size_t index)
	{
		if (args.get().type() != typeid(ast::ExpressionsList))
			return {};

		const auto& expList = boost::get<ast::ExpressionsList>(args);
		if (expList.size() <= index)
			return {};

		const auto& op = expList[index].operand;
		if (op.get().type() != typeid(ast::Numeral))
			return {};

		return boost::get<ast::Numeral>(op);
	}
	
	boost::optional<const std::string&> getLiteralString(const ast::Arguments& args, size_t index)
	{
		if (args.get().type() != typeid(ast::ExpressionsList))
			return {};

		const auto& expList = boost::get<ast::ExpressionsList>(args);
		if (expList.size() <= index)
			return {};

		const auto& op = expList[index].operand;
		if (op.get().type() != typeid(ast::LiteralString))
			return {};

		return boost::get<ast::LiteralString>(op).value;
	}

	an::TypeInfo getType(const an::Scope& scope, const ast::Arguments& args, size_t index)
	{
		if (args.get().type() != typeid(ast::ExpressionsList))
			return an::Type::unknown;

		const auto& expList = boost::get<ast::ExpressionsList>(args);
		if (expList.size() <= index)
			return an::Type::unknown;

		return an::getType(scope, expList[index]);
	}
}
