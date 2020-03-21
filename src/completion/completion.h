#pragma once

#include <parser/chunk.h>

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
	} // namespace comp
} // namespace lac
