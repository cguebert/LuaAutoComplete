#pragma once

#include <lac/parser/chunk.h>

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
	} // namespace comp
} // namespace lac
