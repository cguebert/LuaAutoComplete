#include <editor/LuaEditor.h>

#include <lac/completion/type_at_pos.h>

#include <QToolTip>

namespace lac::editor
{
	LuaEditor::LuaEditor(QWidget* parent)
		: QPlainTextEdit(parent)
	{
	}

	bool LuaEditor::event(QEvent* evt)
	{
		if (evt->type() == QEvent::ToolTip)
		{
			QHelpEvent* helpEvent = static_cast<QHelpEvent*>(evt);
			const auto pos = cursorForPosition(helpEvent->pos()).position();
			const auto text = document()->toPlainText().toStdString();

			const auto typeInfo = lac::comp::getTypeAtPos(text, pos);
			if (typeInfo.type != lac::an::Type::nil)
				QToolTip::showText(helpEvent->globalPos(), QString::fromStdString(typeInfo.typeName()));
			else
				QToolTip::hideText();
			
			return true;
		}

		return QPlainTextEdit::event(evt);
	}
} // namespace lac::editor
