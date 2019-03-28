#include <parser/printer.h>

namespace lac
{
	using namespace ast;

	class Printer : public boost::static_visitor<void>
	{
	public:
		Printer(std::ostream& out, int indent = 0)
			: out(out)
			, indent(indent)
		{
		}

		void operator()(Operation op) const
		{
			static const std::vector<std::string> constants = {
				"+", "-", "*", "/", "//", "%",
				"^", "-", "&", "|", "~", "~",
				"<<", ">>", "..", "#", "<",
				"<=", ">", ">=", "==", "~="};

			out << constants[static_cast<int>(op)];
		}

		void operator()(ExpressionConstant ec) const
		{
			static const std::vector<std::string> constants = {"nil", "...", "false", "true"};
			out << constants[static_cast<int>(ec)];
		}

		void operator()(const std::string& str) const
		{
			out << '"' << str << '"';
		}

		void operator()(double v) const
		{
			out << v;
		}

		void operator()(const Operand& operand) const
		{
			tab();
			out << "Operand: ";
			boost::apply_visitor(Printer(out, indent + 1), operand);
		}

		void operator()(const UnaryOperation& uo) const
		{
			endl();
			tab();
			out << "UnaryOperation: ";
			Printer(out, indent + 1)(uo.operation);
			Printer(out, indent + 1)(uo.expression);
		}

		void operator()(const BinaryOperation& bo) const
		{
			endl(); 
			tab();
			out << "BinaryOperation: ";
			Printer(out, indent + 1)(bo.operation);
			Printer(out, indent + 1)(bo.expression);
		}

		void operator()(const Expression& ex) const
		{
			endl(); 
			tab();
			out << "Expression:\n";
			Printer(out, indent + 1)(ex.operand);
			if(ex.binaryOperation)
				Printer(out, indent + 1)(ex.binaryOperation->get());
		}

		void operator()(const TableConstructor& tc) const
		{
			tab();
			out << "Table constructor";
		}

	private:
		void tab() const
		{
			for (int i = 0; i < indent; ++i)
				out << "  ";
		}

		void endl() const
		{
			out << '\n';
		}

		std::ostream& out;
		int indent = 0;
	};

	void print(const Expression& ex, std::ostream& out)
	{
		Printer{out}(ex);
	}
} // namespace lac
