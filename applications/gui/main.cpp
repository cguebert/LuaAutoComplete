#include <editor/LuaEditor.h>

#include <QApplication>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	lac::editor::LuaEditor editor;
	editor.show();

	return app.exec();
}
