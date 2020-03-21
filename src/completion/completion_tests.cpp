#include <parser/chunk.h>
#include <completion/get_block.h>

#include <helper/test_utils.h>

#include <cctype>

namespace lac
{
	using helper::test_phrase_parser;

	namespace comp
	{
		const std::string program = R"~~(
function split_path(str)
	return split(str,'[\\/]+')
end

parts = split_path("/usr/local/bin")
print(parts)

local function myFunc(testValue)
	if (testValue % 2 == 0) then
		print('even')
	else
		print('odd')
	end

	x = testValue .. 'world'

    t = {}
    t.f = function (firstNum, secondNum) return firstNum + secondNum end
    t.m = testValue

    y = t.f(5, t.m)
end
)~~";

		std::string_view extractText(const ast::PositionAnnotated& pa)
		{
			auto ps = pa.begin, pe = pa.end;
			while (isspace(program[ps]))
				++ps;
			while (isspace(program[pe]))
				--pe;
			std::string_view view = program; 
			return view.substr(ps, pe - ps + 1);
		}

		TEST_SUITE_BEGIN("Completion");

		TEST_CASE("Get block")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser(program, chunkRule(), block));

			auto ptr = pos::getBlockAtPos(block, 30);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(return split(str,'[\\/]+'))~");

			ptr = pos::getBlockAtPos(block, 180);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(print('even'))~");

			ptr = pos::getBlockAtPos(block, 202);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(print('odd'))~");

		//	ptr = pos::getBlockAtPos(block, 310);
		//	REQUIRE(ptr);
		//	CHECK(extractText(*ptr) == R"~(return firstNum + secondNum)~");
		}

		TEST_SUITE_END();
	} // namespace comp
} // namespace lac
