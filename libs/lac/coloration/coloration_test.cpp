#include <lac/coloration/coloration_test.h>

#include <array>
#include <iostream>
#include <vector>

namespace lac
{
	using Color = std::array<uint8_t, 3>;

	using Colors = std::vector<Color>;

	const Colors& getColors()
	{
		static Colors colors;
		if (colors.empty())
		{
			colors.push_back({255, 255, 255});
			colors.push_back({86, 156, 214});
			colors.push_back({214, 157, 13});
			colors.push_back({181, 206, 16});
			colors.push_back({87, 166, 74});
			colors.push_back({213, 187, 19});
			colors.push_back({198, 179, 25});
			colors.push_back({198, 179, 25});
			colors.push_back({163, 214, 16});
		}

		return colors;
	}

	void setColor(ast::ElementType type)
	{
		const auto& c = getColors()[static_cast<size_t>(type)];
		std::cout << "\x1b[38;2;" << (int)c[0]
				  << ';' << (int)c[1]
				  << ';' << (int)c[2]
				  << 'm';
	}

	void resetColor()
	{
		std::cout << "\x1b[30m\033[0m";
	}

	void printProgram(std::string_view text, const lac::pos::Elements& elements)
	{
		size_t pos = 0;
		for (const auto& elt : elements)
		{
			if (elt.begin > pos)
				std::cout << text.substr(pos, elt.begin - pos);
			
			setColor(elt.type);
			std::cout << text.substr(elt.begin, elt.end - elt.begin);
			pos = elt.end;
			setColor(ast::ElementType::not_defined);
		}

		std::cout << text.substr(pos);
		resetColor();
	}
} // namespace lac
