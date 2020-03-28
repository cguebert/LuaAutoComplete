#include <completion/parse_current_line.h>
#include <parser/chunk.h>

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

		auto pe = pos, ps = pos;

		auto goToNameStart = [&ps, &view] {
			while (ps != 0 && isName(view[ps - 1]))
				--ps;
		};

		auto ignoreWhiteSpace = [&ps, &view] {
			while (ps != 0 && std::isspace(view[ps - 1]))
				--ps;
		};

		// Go to the end of the name
		while (pe < view.size() && isName(view[pe]))
			++pe;

		// Go to the start of the name
		goToNameStart();

		// Cannot go to the left
		if (ps < 2)
			return view.substr(ps, pe - ps);

		// Test the next character to the left and see if we must continue
		pos = ps;
		ignoreWhiteSpace();
		auto c = view[ps-1];
		if (c == '.' || c == ':')
		{
			ps -= 2;
			ignoreWhiteSpace();
			goToNameStart();
			pos = ps;
		}

		return view.substr(pos, pe - pos);
	}

	boost::optional<ast::Variable> parseVariableAtPos(std::string_view view, size_t pos)
	{
		auto extracted = extractVariableAtPos(view, pos);
		if (extracted.empty())
			return {};

		ast::Variable var;
		if (parseString(extracted, var))
			return var;
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
		CHECK(extractVariableAtPos("foo.bar", 2) == "foo");
		CHECK(extractVariableAtPos("foo.bar test", 10) == "test");
	}

	TEST_CASE("Parse name")
	{
		auto var = parseVariableAtPos("foobar test", 5);
		REQUIRE(var.has_value());
		REQUIRE(var->start.get().type() == typeid(std::string));
		CHECK(boost::get<std::string>(var->start) == "foobar");
	}

	TEST_CASE("Member variable")
	{
		CHECK(extractVariableAtPos("foo.bar", 5) == "foo.bar");
		CHECK(extractVariableAtPos("foo.bar test", 5) == "foo.bar");
		CHECK(extractVariableAtPos("foo . bar test", 7) == "foo . bar");
		CHECK(extractVariableAtPos("test foo.bar", 10) == "foo.bar");
	}

	TEST_CASE("Member function")
	{
		CHECK(extractVariableAtPos("foo:bar", 5) == "foo:bar");
		CHECK(extractVariableAtPos("foo:bar test", 5) == "foo:bar");
		CHECK(extractVariableAtPos("foo : bar test", 7) == "foo : bar");
		CHECK(extractVariableAtPos("test foo:bar", 10) == "foo:bar");
	}

	TEST_SUITE_END();
} // namespace lac::pos
