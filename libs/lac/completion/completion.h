#pragma once

#include <lac/parser/ast.h>
#include <lac/parser/positions.h>
#include <lac/analysis/scope.h>

namespace lac
{
	namespace comp
	{
		class Completion
		{
		public:
			bool updateProgram(std::string_view str, size_t currentPosition);

		private:
			ast::Block m_rootBlock;
			pos::Positions<std::string_view::const_iterator> m_positions;
		};

		// Remove the last member of the variable. If not possible, return empty.
		boost::optional<ast::VariableOrFunction> removeLastPart(ast::VariableOrFunction var);

		// Return the "parent" variable at the current position
		boost::optional<ast::VariableOrFunction> getContext(std::string_view str, size_t pos = std::string_view::npos);

		// Return a list of possibilities for auto-completion
		an::ElementsMap getAutoCompletionList(const an::Scope& localScope, const boost::optional<ast::VariableOrFunction>& var);

	} // namespace comp
} // namespace lac
