#include <analysis/scope.h>
#include <analysis/visitor.h>
#include <completion/get_block.h>
#include <helper/test_utils.h>
#include <parser/chunk.h>

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
			const auto blocks = pos::getChildren(block);

			auto ptr = pos::getBlockAtPos(blocks, 30);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(return split(str,'[\\/]+'))~");

			ptr = pos::getBlockAtPos(blocks, 180);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(print('even'))~");

			ptr = pos::getBlockAtPos(blocks, 202);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(print('odd'))~");

			ptr = pos::getBlockAtPos(blocks, 310);
			REQUIRE(ptr);
			CHECK(extractText(*ptr) == R"~(return firstNum + secondNum)~");
		}

		TEST_CASE("Get scope")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser(program, chunkRule(), block));
			const auto scope = an::analyseBlock(block);

			auto ptr = pos::getScopeAtPos(scope, 30);
			REQUIRE(ptr);
			REQUIRE(ptr->block());
			CHECK(extractText(*ptr->block()) == R"~(return split(str,'[\\/]+'))~");

			ptr = pos::getScopeAtPos(scope, 180);
			REQUIRE(ptr);
			REQUIRE(ptr->block());
			CHECK(extractText(*ptr->block()) == R"~(print('even'))~");

			ptr = pos::getScopeAtPos(scope, 202);
			REQUIRE(ptr);
			REQUIRE(ptr->block());
			CHECK(extractText(*ptr->block()) == R"~(print('odd'))~");

			ptr = pos::getScopeAtPos(scope, 310);
			REQUIRE(ptr);
			REQUIRE(ptr->block());
			CHECK(extractText(*ptr->block()) == R"~(return firstNum + secondNum)~");
		}

		TEST_CASE("Scope elements")
		{
			ast::Block block;
			REQUIRE(test_phrase_parser(program, chunkRule(), block));
			const auto scope = an::analyseBlock(block);

			auto ptr = pos::getScopeAtPos(scope, 30);
			REQUIRE(ptr);
			auto elements = ptr->getElements();
			CHECK(elements.size() == 1);
			CHECK(elements.count("str"));

			ptr = pos::getScopeAtPos(scope, 310);
			REQUIRE(ptr);
			elements = ptr->getElements();
			CHECK(elements.size() == 2);
			CHECK(elements.count("firstNum"));
			CHECK(elements.count("secondNum"));

			ptr = pos::getScopeAtPos(scope, 350);
			REQUIRE(ptr);
			elements = ptr->getElements();
			CHECK(elements.size() == 4);
			CHECK(elements.count("testValue"));
			CHECK(elements.count("x"));
			CHECK(elements.count("y"));
			REQUIRE(elements.count("t"));

			const auto& t = elements.at("t");
			CHECK(t.elementType == an::ElementType::variable);
			CHECK(t.typeInfo.type == an::Type::table);
			CHECK(t.typeInfo.member("f").type == an::Type::function);
			CHECK(t.typeInfo.member("m").type != an::Type::nil);
			CHECK(t.typeInfo.member("n").type == an::Type::nil);
		}

		TEST_SUITE_END();
	} // namespace comp
} // namespace lac
