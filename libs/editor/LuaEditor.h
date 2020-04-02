#pragma once

#include <editor/editor_api.h>

#include <QPlainTextEdit>

namespace lac::editor
{
	class EDITOR_API LuaEditor : public QPlainTextEdit
	{
		Q_OBJECT
	public:
		LuaEditor(QWidget* parent = nullptr);

	protected:
		bool event(QEvent* evt) override;
	};
} // namespace lac::editor
