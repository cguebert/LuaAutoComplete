#include <lac/completion/completion.h>
#include <lac/completion/get_block.h>
#include <lac/parser/parser.h>

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
