#include <lac/analysis/get_type.h>
#include <lac/analysis/get_sub_type.h>
#include <lac/analysis/scope.h>
#include <lac/parser/ast.h>

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

		TypeInfo operator()(const ast::Numeral&) const
		{
			return Type::number;
		}

		TypeInfo operator()(const ast::LiteralString&) const
		{
			return Type::string;
		}

		TypeInfo operator()(const std::string& str) const
		{
			TypeInfo type = Type::unknown;
			type.name = str;
			return type;
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
			TypeInfo info{Type::table};
			if (!tc.fields)
				return info;

			int fieldIndex = 1;
			for (const ast::Field& field : *tc.fields)
			{
				const auto& fieldType = field.get().type();
				if (fieldType == typeid(ast::FieldByAssignment))
				{
					const auto& assignment = boost::get<ast::FieldByAssignment>(field.get());
					info.members[assignment.name] = getType(m_scope, assignment.value);
				}
				else if (fieldType == typeid(ast::Expression))
				{
					const auto& expression = boost::get<ast::Expression>(field.get());
					const auto fieldName = std::to_string(fieldIndex++);
					info.members[fieldName] = getType(m_scope, expression);
				}
				// TODO support field by expression
			}

			return info;
		}

		TypeInfo operator()(const ast::FunctionBody& fb) const
		{
			TypeInfo info{Type::function};
			if (fb.parameters && !fb.parameters->parameters.empty())
			{
				const auto& params = fb.parameters->parameters;
				size_t i = 0;
				if (params.front() == "self")
				{
					info.function.isMethod = true;
					++i; // Ignore this parameter
				}

				for (size_t nb = params.size(); i<nb; ++i)
					info.function.parameters.emplace_back(params[i]);
			}
			// TODO: fill the function results
			return info;
		}

		TypeInfo operator()(const ast::BracketedExpression& be) const
		{
			return (*this)(be.expression);
		}

		TypeInfo operator()(const ast::PrefixExpression& pe) const
		{
			auto type = boost::apply_visitor(*this, pe.start);
			if (pe.rest.empty())
				return m_scope.getVariableType(type.name);

			for (const auto& r : pe.rest)
				type = getSubType(m_scope, type, r);
			return type;
		}

		TypeInfo operator()(const ast::Variable& v) const
		{
			auto type = boost::apply_visitor(*this, v.start);
			if (v.rest.empty())
				return m_scope.getVariableType(type.name);

			for (const auto& r : v.rest)
				type = getSubType(m_scope, type, r);
			return type;
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

	TypeInfo getType(const Scope& scope, const ast::FunctionBody& f)
	{
		return GetType{scope}(f);
	}
} // namespace lac::an
