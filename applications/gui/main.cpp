#include <editor/LuaEditor.h>

#include <QApplication>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	lac::editor::LuaEditor editor;
	editor.setMinimumSize(600, 400);
	editor.setPlainText(R"~~(--Test program
num = 42
text = 'foo'
test = true
func = function(a, b)
	return a + b
end

myTable = {}
myTable.num = 69
myTable.text = 'bar'
myTable.bool = false
myTable.neg = function(a) return -a end
myTable.mult = function(a) return a * 2 end
)~~");
	editor.show();

	return app.exec();
}
