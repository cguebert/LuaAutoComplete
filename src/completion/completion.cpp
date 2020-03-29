#include <completion/completion.h>
#include <completion/get_block.h>
#include <parser/parser.h>

namespace lac::comp
{
	bool Completion::updateProgram(std::string_view view, size_t currentPosition)
	{
		auto ret = lac::parser::parseBlock(view);

		if (ret.parsed)
		{
			std::swap(m_rootBlock, ret.block);
			std::swap(m_positions, ret.positions);
		}

		return ret.parsed;
	}
} // namespace lac::comp
