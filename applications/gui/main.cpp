#include <editor/LuaEditor.h>
#include <editor/EditorHighlighter.h>

#include <lac/analysis/user_defined.h>

#include <QApplication>

lac::an::UserDefined createUserDefined()
{
	using namespace lac::an;
	UserDefined userDefined;
	TypeInfo mathType = Type::table;
	mathType.members["abs"] = TypeInfo::createFunction({{"x", Type::number}}, {Type::number});
	mathType.members["ceil"] = TypeInfo::createFunction({{"x", Type::number}}, {Type::number});
	mathType.members["floor"] = TypeInfo::createFunction({{"x", Type::number}}, {Type::number});
	mathType.members["max"] = TypeInfo::createFunction({{"x", Type::number}, {"y", Type::number}}, {Type::number});
	mathType.members["min"] = TypeInfo::createFunction({{"x", Type::number}, {"y", Type::number}}, {Type::number});
	userDefined.addVariable("math", std::move(mathType));

	UserType playerType;
	playerType.name = "Player";
	playerType.variables["x"] = Type::number;
	playerType.variables["y"] = Type::number;
	playerType.variables["name"] = Type::string;
	userDefined.addType(playerType);
	userDefined.addScriptInput("run", {{{"player", TypeInfo::fromTypeName("Player")}}, {}});

	return userDefined;
}

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	auto userDefined = createUserDefined();

	lac::editor::LuaEditor editor;
	editor.setMinimumSize(600, 600);
	editor.setTabStopDistance(30);
	editor.setUserDefined(&userDefined);
	editor.highlighter()->useLuaRules();
	editor.setPlainText(R"~~(--Test program
num = 42
text = 'foo'
longText = [[multi-line
text]]
test = true
func = function(a, b)
	return a + b
end

--[[ Multi-line
	comment ]]

myTable = {}
myTable.num = 69
myTable.text = "bar"
myTable.bool = false
myTable.neg = function(a) return -a end
myTable.mult = function(a) return a * 2 end

function run (player)
	
end

-- Testing user defined types
x = math.abs(-3.14)
)~~");
	editor.show();

	return app.exec();
}
