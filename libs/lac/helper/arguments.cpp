#include <lac/helper/arguments.h>

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
}
