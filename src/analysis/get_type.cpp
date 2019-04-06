#include <analysis/get_type.h>

namespace lac::an
{
	class GetType : public boost::static_visitor<TypeInfo>
	{
	public:
		GetType(const Scope& scope)
			: m_scope(scope)
		{
		}

		TypeInfo operator()(ast::ExpressionConstant ec) const
		{
			using EC = ast::ExpressionConstant;
			switch (ec)
			{
			case EC::nil:
			case EC::dots:
				return Type::nil;

			case EC::False:
			case EC::True:
				return Type::boolean;
			}

			return Type::nil;
		}

		TypeInfo operator()(int v) const
		{
			return Type::number;
		}

		TypeInfo operator()(double v) const
		{
			return Type::number;
		}

		TypeInfo operator()(const std::string& str) const
		{
			return Type::string;
		}

		TypeInfo operator()(const ast::UnaryOperation& uo) const
		{
			using OP = ast::Operation;
			switch (uo.operation)
			{
			case OP::unm:
			case OP::len:
			case OP::bnot:
				return Type::number;

			case OP::lnot:
				return Type::boolean;
			}

			return Type::nil;
		}

		TypeInfo operator()(const ast::BinaryOperation& bo) const
		{
			return {}; // TODO: do something here
		}

		TypeInfo operator()(const ast::TableConstructor& tc) const
		{
			// TODO: fill the table members
			return Type::table;
		}

		TypeInfo operator()(const ast::FunctionBody& fb) const
		{
			// TODO: fill the function parameters and results
			return Type::function;
		}

		TypeInfo operator()(const ast::PrefixExpression& pe) const
		{
			return {}; // TODO: do something here
		}

		TypeInfo operator()(const ast::Expression& e)
		{
			const auto first = boost::apply_visitor(*this, e.operand);
			if (!e.binaryOperation)
				return first;
			return {}; // TODO: do something here
		}

	private:
		const Scope& m_scope;
	};

	TypeInfo getType(const Scope& scope, const ast::Expression& e)
	{
		return GetType{scope}(e);
	}
} // namespace lac::an
