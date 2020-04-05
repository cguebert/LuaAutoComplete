#pragma once

#include <editor/editor_api.h>
#include <lac/completion/completion.h>

#include <QPlainTextEdit>

class QCompleter;

namespace lac::editor
{
	class CompletionModel;

	class EDITOR_API LuaEditor : public QPlainTextEdit
	{
		Q_OBJECT
	public:
		LuaEditor(QWidget* parent = nullptr);

	protected:
		bool event(QEvent* evt) override;
		void keyPressEvent(QKeyEvent* event) override;

		bool handleTabs(QKeyEvent* event);
		void handleCompletion(QKeyEvent* event);
		void updateProgram();
		void completeWord(const QString& word);

	private:
		QCompleter* m_completer = nullptr;
		CompletionModel* m_completionModel = nullptr;
		lac::comp::Completion m_programCompletion;

		bool m_textChanged = true;
	};
} // namespace lac::editor
