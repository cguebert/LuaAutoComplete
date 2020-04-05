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
myTable.m_num = 69
myTable.m_text = 'bar'
myTable.m_bool = false
myTable.m_func1 = function(a) return -a end
myTable.m_func2 = function(a) return a * 2 end
)~~");
	editor.show();

	return app.exec();
}
