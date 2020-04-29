#include "coloration_test.h"

#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include <doctest/doctest.h>

#include <lac/parser/ast_adapted.h>
#include <lac/parser/parser.h>
#include <lac/parser/positions.h>
#include <lac/parser/printer.h>

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

		const auto ret = lac::parser::parseBlock(data);
		if (ret.parsed)
			color::printProgram(data, ret.positions.elements());
	}

	void printAst(std::string_view path)
	{
		const auto data = loadFile(path);
		if (data.empty())
			return;

		const auto ret = lac::parser::parseBlock(data, false);
		if (ret.parsed)
#ifdef WITH_NLOHMANN_JSON
			std::cout << lac::toJson(ret.block);
#else
			std::cout << "json library not available\n";
#endif
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
