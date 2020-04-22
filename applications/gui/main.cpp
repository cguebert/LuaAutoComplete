#include <lac/editor/LuaEditor.h>
#include <lac/editor/EditorHighlighter.h>

#include <lac/analysis/user_defined.h>

#include <QApplication>

lac::an::UserDefined createUserDefined()
{
	using namespace lac::an;
	UserDefined userDefined;
	TypeInfo mathType = Type::table;
	mathType.members["abs"] = "number function(number val)";
	mathType.members["ceil"] = "number function(number val)";
	mathType.members["floor"] = "number function(number val)";
	mathType.members["max"] = "number function(number x, number y)";
	mathType.members["min"] = "number function(number x, number y)";
	userDefined.addVariable("math", std::move(mathType));

	TypeInfo vec3Type = Type::table;
	vec3Type.name = "Vector3";
	vec3Type.members["x"] = Type::number;
	vec3Type.members["y"] = Type::number;
	vec3Type.members["z"] = Type::number;
	vec3Type.members["length"] = "number method()";
	vec3Type.members["mult"] = "Vector3 method(number v)";
	userDefined.addType(std::move(vec3Type));

	TypeInfo playerType = Type::table;
	playerType.name = "Player";
	playerType.members["position"] = "Vector3 method()";
	playerType.members["setPosition"] = "method(Vector3 position)";
	playerType.members["name"] = Type::string;
	userDefined.addType(playerType);
	userDefined.addScriptInput("run", "function(Player player)");

	return userDefined;
}

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	auto userDefined = createUserDefined();

	lac::editor::LuaEditor editor;
	editor.setMinimumSize(600, 600);
	editor.setTabStopDistance(30);
	editor.setUserDefined(userDefined);
	editor.highlighter()->useLuaRules();
	editor.setPlainText(R"~~(--Test program
num = 42
text = 'foo'
longText = [[multi-line
text with keywords and 42, 3.1459, 'str' ]]
test = true
func = function(a, b)
	return a + b
end

--[[ Multi-line
	comment, 42, while, "text" ]]

myTable = {}
myTable.num = 69
myTable.text = "bar for 42"
myTable.bool = false
myTable.neg = function(a) return -a end
myTable.mult = function(a) return a * 2 end

function run (player)
	local pos = player:position()
	local len = pos:length()
	if len > 500 then
		pos = pos:mult(500 / len)
		player:setPosition(pos)
	end
end

-- Testing user defined types
x = math.abs(-3.14)
)~~");
	editor.show();

	return app.exec();
}
