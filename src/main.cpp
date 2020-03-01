#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <coloration/coloration_test.h>
#include <parser/ast_adapted.h>
#include <parser/chunk.h>
#include <parser/config.h>
#include <parser/positions.h>
#include <parser/printer.h>

#include <fstream>

#ifdef WIN32
#include <windows.h>
void activateColorConsole()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return;

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
		return;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
		return;
}
#else
void activateColorConsole()
{
}
#endif

namespace
{
	std::string loadFile(std::string_view path)
	{
		std::ifstream in(path.data(), std::ios_base::binary);
		if (!in)
			return {};

		std::string data;
		in.seekg(0, std::ios_base::end);
		data.resize(in.tellg());
		in.seekg(0, std::ios_base::beg);
		in.read(data.data(), data.size());
		return data;
	}

	void colorProgram(std::string_view path)
	{
		activateColorConsole();
		const auto data = loadFile(path);
		if (data.empty())
			return;

		const auto view = std::string_view{data};
		auto f = view.begin();
		const auto l = view.end();
		lac::pos::Positions positions{f, l};
		lac::ast::Block block;

		if (lac::parseString(view, positions, block))
			lac::printProgram(data, positions.elements());
	}

	void printAst(std::string_view path)
	{
		const auto data = loadFile(path);
		if (data.empty())
			return;

		const auto view = std::string_view{data};
		auto f = view.begin();
		const auto l = view.end();
		lac::ast::Block block;

		if (lac::parseString(view, block))
			std::cout << lac::toJson(block).dump(2, ' ');
	}
} // namespace

int main(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
	{
		const std::string cmd = argv[i];
		if (cmd == "color")
		{
			colorProgram(argv[++i]);
			return 0;
		}
		else if (cmd == "print_ast")
		{
			printAst(argv[++i]);
			return 0;
		}
	}

	doctest::Context context;
	context.applyCommandLine(argc, argv);

	int res = context.run();

	if (context.shouldExit() || res)
		return res;

	return 0;
}
