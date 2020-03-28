#include <completion/parse_current_line.h>

#include <doctest/doctest.h>
#include <cctype>

namespace
{
	bool isName(int ch)
	{
		return std::isalnum(ch) || ch == '_';
	}
} // namespace

namespace lac::pos
{
	std::string_view extractVariableAtPos(std::string_view view, size_t pos)
	{
		if (pos >= view.size() || !isName(view[pos]))
			return {};

		// Go to the end of the name
		auto pe = pos;
		while (pe < view.size() && isName(view[pe]))
			++pe;

		// Go to the start of the name
		auto ps = pos;
		while (ps != 0 && isName(view[ps - 1]))
			--ps;

		return view.substr(ps, pe - ps);
	}

	boost::optional<ast::Variable> parseVariableAtPos(std::string_view view, size_t pos)
	{
		auto extracted = extractVariableAtPos(view, pos);
		if (extracted.empty())
			return {};

		return {};
	}

	TEST_SUITE_BEGIN("Parse current line");

	TEST_CASE("Name only")
	{
		CHECK(extractVariableAtPos("foobar", 0) == "foobar");
		CHECK(extractVariableAtPos("foobar", 3) == "foobar");
		CHECK(extractVariableAtPos("foobar", 5) == "foobar");
		CHECK(extractVariableAtPos("foobar test", 5) == "foobar");
		CHECK(extractVariableAtPos("foobar test", 6) == "");
		CHECK(extractVariableAtPos("foobar test", 7) == "test");
		CHECK(extractVariableAtPos("foobar test", 10) == "test");
		CHECK(extractVariableAtPos("foobar test", 11) == "");
	}

	TEST_SUITE_END();
} // namespace lac::pos
