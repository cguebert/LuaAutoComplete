#include <completion/completion.h>
#include <completion/get_block.h>

namespace lac::comp
{
	bool Completion::updateProgram(std::string_view view, size_t currentPosition)
	{
		lac::ast::Block block;
		auto f = view.begin();
		const auto l = view.end();
		lac::pos::Positions positions{f, l};
		auto ret = lac::parseString(view, positions, block);

		if (ret)
		{
			std::swap(m_rootBlock, block);
			std::swap(m_positions, positions);
		}

		return ret;
	}
} // namespace lac::comp
