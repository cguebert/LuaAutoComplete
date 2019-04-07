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
				return Type::nil;

			case EC::dots:
				return Type::unknown;

			case EC::False:
			case EC::True:
				return Type::boolean;

			default:
				return Type::error;
			}
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
			const auto right = (*this)(uo.expression);

			using OP = ast::Operation;
			switch (uo.operation)
			{
			case OP::unm:
			case OP::bnot:
				return right.convert(Type::number);

			case OP::len:
				return (right.type == Type::string || right.type == Type::table)
						   ? Type::number
						   : Type::error;

			case OP::lnot:
				return Type::boolean;

			default:
				return Type::error;
			}
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

		TypeInfo operator()(const ast::Expression& e) const
		{
			const auto first = boost::apply_visitor(*this, e.operand);
			if (!e.binaryOperation)
				return first;

			return binaryOperation(first, *e.binaryOperation);
		}

		TypeInfo binaryOperation(const TypeInfo& left, const ast::BinaryOperation& bo) const
		{
			// TODO: we cannot correctly support left associative operations
			//  (must rework the parser)

			const auto operand = (*this)(bo.expression);

			auto convertAll = [&](Type type) {
				if (left.convert(type) && operand.convert(type))
					return type;
				return Type::error;
			};

			auto allType = [&](Type type) {
				return left.type == type && operand.type == type;
			};

			using OP = ast::Operation;
			switch (bo.operation)
			{
			case OP::add:
			case OP::sub:
			case OP::mul:
			case OP::div:
			case OP::idiv:
			case OP::mod:
				return convertAll(Type::number);

			case OP::pow: // Right associative
				return convertAll(Type::number);

			case OP::band:
			case OP::bor:
			case OP::bxor:
			case OP::bnot:
			case OP::shl:
			case OP::shr:
				return convertAll(Type::number);

			case OP::concat: // Right associative
				return convertAll(Type::string);

			case OP::lt:
			case OP::le:
			case OP::gt:
			case OP::ge:
				if (allType(Type::number) || allType(Type::string))
					return Type::boolean;
				return Type::error;

			case OP::eq:
			case OP::ineq:
				return Type::boolean;

			case OP::land:
			case OP::lor:
				// TODO: create a variant type
				if (!left && !operand)
					return Type::error;
				if (left.type == operand.type)
					return left;
				return Type::unknown;

			default:
				return Type::error;
			}
		}

	private:
		const Scope& m_scope;
	};

	TypeInfo getType(const Scope& scope, const ast::Expression& e)
	{
		return GetType{scope}(e);
	}
} // namespace lac::an
