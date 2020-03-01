#include <parser/printer.h>

namespace lac
{
	using namespace ast;

	class Printer : public boost::static_visitor<nlohmann::json>
	{
	public:
		std::string getString(Operation op) const
		{
			static const std::vector<std::string> constants = {
				"+", "-", "*", "/", "//", "%",
				"^", "-", "&", "|", "~", "~",
				"<<", ">>", "..", "#", "<",
				"<=", ">", ">=", "==", "~="};

			return constants[static_cast<int>(op)];
		}

		nlohmann::json operator()(ExpressionConstant ec) const
		{
			static const std::vector<std::string> constants = {"nil", "...", "false", "true"};

			nlohmann::json j;
			j["type"] = "ExpressionConstant";
			j["contant"] = constants[static_cast<int>(ec)];
			return j;
		}

		nlohmann::json operator()(const LiteralString& ls) const
		{
			nlohmann::json j;
			j["type"] = "LiteralString";
			j["value"] = ls.value;
			return j;
		}

		nlohmann::json operator()(int v) const
		{
			nlohmann::json j;
			j["type"] = "Numeral";
			j["value"] = v;
			return j;
		}

		nlohmann::json operator()(double v) const
		{
			nlohmann::json j;
			j["type"] = "Numeral";
			j["value"] = v;
			return j;
		}

		nlohmann::json operator()(std::string v) const
		{
			nlohmann::json j;
			j["type"] = "Name";
			j["value"] = v;
			return j;
		}

		nlohmann::json operator()(const NamesList& list) const
		{
			nlohmann::json j;
			j["type"] = "NamesList";
			if (!list.empty())
			{
				auto& names = j["names"];
				for (const auto& name : list)
					names.push_back(name);
			}
			return j;
		}

		nlohmann::json operator()(const ast::Numeral& n) const
		{
			return boost::apply_visitor(Printer{}, n);
		}

		nlohmann::json operator()(const Operand& operand) const
		{
			return boost::apply_visitor(Printer{}, operand);
		}

		nlohmann::json operator()(const UnaryOperation& uo) const
		{
			nlohmann::json j;
			j["type"] = "UnaryOperation";
			j["operator"] = getString(uo.operation);
			j["argument"] = (*this)(uo.expression);
			return j;
		}

		nlohmann::json operator()(const BinaryOperation& bo) const
		{
			nlohmann::json j;
			j["type"] = "BinaryOperation";
			j["operator"] = getString(bo.operation);
			j["argument"] = (*this)(bo.expression);
			return j;
		}

		nlohmann::json operator()(const BracketedExpression& be) const
		{
			nlohmann::json j;
			j["type"] = "BracketedExpression";
			j["expression"] = (*this)(be.expression);
			return j;
		}

		nlohmann::json operator()(const TableIndexExpression& tie) const
		{
			nlohmann::json j;
			j["type"] = "TableIndexExpression";
			j["expression"] = (*this)(tie.expression);
			return j;
		}

		nlohmann::json operator()(const TableIndexName& tin) const
		{
			nlohmann::json j;
			j["type"] = "TableIndexName";
			j["name"] = tin.name;
			return j;
		}

		nlohmann::json operator()(const ExpressionsList& list) const
		{
			nlohmann::json j;
			j["type"] = "ExpressionsList";
			auto& expressions = j["expressions"];
			for (const auto& ex : list)
				expressions.push_back((*this)(ex));
			return j;
		}

		nlohmann::json operator()(const EmptyArguments& ar) const
		{
			nlohmann::json j;
			j["type"] = "EmptyArguments";
			return j;
		}

		nlohmann::json operator()(const Arguments& ar) const
		{
			return boost::apply_visitor(Printer{}, ar);
		}

		nlohmann::json operator()(const FunctionCallEnd& fce) const
		{
			nlohmann::json j;
			j["type"] = "FunctionCallEnd";
			if (fce.member)
				j["member"] = *fce.member;
			j["arguments"] = (*this)(fce.arguments);
			return j;
		}

		nlohmann::json operator()(const VariableFunctionCall& vfc) const
		{
			nlohmann::json j;
			j["type"] = "VariableFunctionCall";
			j["call"] = (*this)(vfc.functionCall);
			j["post"] = (*this)(vfc.postVariable);
			return j;
		}

		nlohmann::json operator()(const VariablePostfix& post) const
		{
			return boost::apply_visitor(Printer{}, post);
		}

		nlohmann::json operator()(const Variable& v) const
		{
			nlohmann::json j;
			j["type"] = "Variable";
			j["start"] = boost::apply_visitor(Printer{}, v.start);
			if (!v.rest.empty())
			{
				auto& rest = j["rest"];
				for (const auto& post : v.rest)
					rest.push_back((*this)(post));
			}
			return j;
		}

		nlohmann::json operator()(const Expression& ex) const
		{
			nlohmann::json j;
			j["type"] = "Expression";
			j["left"] = (*this)(ex.operand);
			if (ex.binaryOperation)
				j["right"] = (*this)(*ex.binaryOperation);
			return j;
		}

		nlohmann::json operator()(const FieldByExpression& fbe) const
		{
			nlohmann::json j;
			j["type"] = "FieldByExpression";
			j["key"] = (*this)(fbe.key);
			j["value"] = (*this)(fbe.value);
			return j;
		}

		nlohmann::json operator()(const FieldByAssignment& fba) const
		{
			nlohmann::json j;
			j["type"] = "FieldByAssignment";
			j["name"] = (*this)(fba.name);
			j["value"] = (*this)(fba.value);
			return j;
		}

		nlohmann::json operator()(const Field& field) const
		{
			return boost::apply_visitor(Printer{}, field);
		}

		nlohmann::json operator()(const FieldsList& list) const
		{
			nlohmann::json j;
			j["type"] = "FieldsList";
			if (!list.empty())
			{
				auto& fields = j["fields"];
				for (const auto& field : list)
					fields.push_back((*this)(field));
			}
			return j;
		}

		nlohmann::json operator()(const TableConstructor& tc) const
		{
			nlohmann::json j;
			j["type"] = "TableConstructor";
			if (tc.fields)
				j["fields"] = (*this)(*tc.fields);
			return j;
		}

		nlohmann::json operator()(const PostPrefix& post) const
		{
			return boost::apply_visitor(Printer{}, post);
		}

		nlohmann::json operator()(const PrefixExpression& pe) const
		{
			nlohmann::json j;
			j["type"] = "PrefixExpression";
			j["start"] = boost::apply_visitor(Printer{}, pe.start);
			if (!pe.rest.empty())
			{
				auto& rest = j["rest"];
				for (const auto& post : pe.rest)
					rest.push_back((*this)(post));
			}
			return j;
		}

		nlohmann::json operator()(const ParametersList& list) const
		{
			nlohmann::json j;
			j["type"] = "ParametersList";
			if (!list.parameters.empty())
			{
				auto& parameters = j["parameters"];
				for (const auto& par : list.parameters)
					parameters.push_back(par);
			}
			j["varargs"] = list.varargs;
			return j;
		}

		nlohmann::json operator()(const FunctionBody& fb) const
		{
			nlohmann::json j;
			j["type"] = "Function body";
			if (fb.parameters)
				j["parameters"] = (*this)(*fb.parameters);
			j["body"] = (*this)(fb.block);
			return j;
		}

		nlohmann::json operator()(const EmptyStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "EmptyStatement";
			return j;
		}

		nlohmann::json operator()(const AssignmentStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "AssignmentStatement";
			auto& left = j["left"];
			for (const auto& v : s.variables)
				left.push_back((*this)(v));

			auto& right = j["right"];
			for (const auto& sx : s.expressions)
				right.push_back((*this)(sx));
			return j;
		}

		nlohmann::json operator()(const FunctionCall& s) const
		{
			nlohmann::json j;
			j["type"] = "FunctionCall";
			return j;
		}

		nlohmann::json operator()(const LabelStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "LabelStatement";
			j["name"] = s.name;
			return j;
		}

		nlohmann::json operator()(const GotoStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "GotoStatement";
			j["label"] = s.label;
			return j;
		}

		nlohmann::json operator()(const BreakStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "BreakStatement";
			return j;
		}

		nlohmann::json operator()(const DoStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "DoStatement";
			j["body"] = (*this)(s.block);
			return j;
		}

		nlohmann::json operator()(const WhileStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "WhileStatement";
			j["test"] = (*this)(s.condition);
			j["body"] = (*this)(s.block);
			return j;
		}

		nlohmann::json operator()(const RepeatStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "RepeatStatement";
			j["test"] = (*this)(s.condition);
			j["body"] = (*this)(s.block);
			return j;
		}

		nlohmann::json operator()(const IfStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "IfStatement";
			j["test"] = (*this)(s.condition);
			j["body"] = (*this)(s.block);
			return j;
		}

		nlohmann::json operator()(const IfThenElseStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "IfThenElseStatement";
			j["first"] = (*this)(s.first);
			if (!s.rest.empty())
			{
				auto& rest = j["rest"];
				for (const auto& ifs : s.rest)
					rest.push_back((*this)(ifs));
			}
			if (s.elseBlock)
				j["alternate"] = (*this)(*s.elseBlock);
			return j;
		}

		nlohmann::json operator()(const NumericalForStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "NumericalForStatement";
			j["variable"] = s.variable;
			j["first"] = (*this)(s.first);
			j["last"] = (*this)(s.last);
			if (s.step)
				j["step"] = (*this)(*s.step);
			j["body"] = (*this)(s.block);
			return j;
		}

		nlohmann::json operator()(const GenericForStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "GenericForStatement";
			j["variables"] = (*this)(s.variables);
			j["expressions"] = (*this)(s.expressions);
			j["body"] = (*this)(s.block);
			return j;
		}

		nlohmann::json operator()(const FunctionName& fn) const
		{
			nlohmann::json j;
			j["type"] = "FunctionName";
			j["start"] = fn.start;
			if (!fn.rest.empty())
			{
				auto& rest = j["rest"];
				for (const auto& n : fn.rest)
					rest.push_back(n);
			}
			if (fn.member)
				j["member"] = fn.member->name;
			return j;
		}

		nlohmann::json operator()(const FunctionDeclarationStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "FunctionDeclarationStatement";
			j["name"] = (*this)(s.name);
			j["body"] = (*this)(s.body);
			return j;
		}

		nlohmann::json operator()(const LocalFunctionDeclarationStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "LocalFunctionDeclarationStatement";
			j["name"] = (*this)(s.name);
			j["body"] = (*this)(s.body);
			return j;
		}

		nlohmann::json operator()(const LocalAssignmentStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "LocalAssignmentStatement";
			auto& left = j["left"];
			for (const auto& v : s.variables)
				left.push_back((*this)(v));

			if (s.expressions)
			{
				auto& right = j["right"];
				for (const auto& sx : *s.expressions)
					right.push_back((*this)(sx));
			}
			return j;
		}

		nlohmann::json operator()(const Statement& s) const
		{
			return boost::apply_visitor(Printer{}, s);
		}

		nlohmann::json operator()(const ReturnStatement& s) const
		{
			nlohmann::json j;
			j["type"] = "ReturnStatement";
			auto& argument = j["argument"];
			if (s.expressions.empty())
				argument.push_back(nullptr);
			else
			{
				for (const auto& ex : s.expressions)
					argument.push_back((*this)(ex));
			}
			return j;
		}

		nlohmann::json operator()(const Block& b) const
		{
			nlohmann::json j;
			j["type"] = "Block";
			auto& body = j["body"];
			for (const auto& statement : b.statements)
				body.push_back((*this)(statement));

			if (b.returnStatement)
				body.push_back((*this)(*b.returnStatement));
			return j;
		}
	};

	nlohmann::json toJson(const ast::Expression& ex)
	{
		return Printer{}(ex);
	}

	nlohmann::json toJson(const ast::Block& block)
	{
		return Printer{}(block);
	}
} // namespace lac
