#pragma once

#include <lac/editor_api.h>
#include <lac/completion/completion.h>
#include <lac/parser/parser.h>

#include <QPlainTextEdit>

class QCompleter;

namespace lac::editor
{
	class CompletionModel;
	class EditorHighlighter;

	struct EditorDesign
	{
		QColor editor_back_color = "#1e1e1e";
		QColor editor_text_color = "#dcdcdc";
		QColor editor_border_color = "#b9b9b9";
		QColor completion_back_color = "#303030";
		QColor completion_text_color = "#ffffff";
		QColor completion_border_color = "#b9b9b9";
		QColor completion_selection_back_color = "#c0dcf3";
		QColor completion_selection_text_color = "#ffffff";
		QColor completion_selection_border_color = "#90c8f6";
		
		QFont editor_font{"Consolas", 10};
		QFont completion_font{"Consolas", 10};
		QMargins editor_border = {1, 1, 1, 1};
		QMargins completion_border = {1, 1, 1, 1};
	};

	class EDITOR_API LuaEditor : public QPlainTextEdit
	{
		Q_OBJECT
	public:
		using TooltipFunc = std::function<QString(const lac::an::TypeInfo& type)>;

		LuaEditor(QWidget* parent = nullptr);

		void setDesign(const EditorDesign& design); // Change the style of the editor

		lac::an::UserDefined userDefined() const;
		void setUserDefined(lac::an::UserDefined userDefined); // Setup custom types & functions

		EditorHighlighter* highlighter();

		void setTooltipFunc(TooltipFunc func);

		an::TypeInfo getTypeAtCursor();
		std::vector<std::string> getTypeHierarchyAtCursor();
		std::string getVariableNameAtCursor();

		parser::ParseBlockResults compileProgram();

	protected:
		bool event(QEvent* evt) override;
		void keyPressEvent(QKeyEvent* event) override;

		bool handleTabs(QKeyEvent* event);
		void handleCompletion(QKeyEvent* event);
		void completeWord(const QString& word);

		enum class CompletionType
		{
			None,
			Variable,
			Argument
		};
		void updateProgram(CompletionType type = CompletionType::None);

	private:
		QCompleter* m_completer = nullptr;
		CompletionModel* m_completionModel = nullptr;
		lac::comp::Completion m_programCompletion;

		EditorHighlighter* m_highlighter = nullptr;
		bool m_textChanged = true;

		TooltipFunc m_tooltipFunc;
	};
} // namespace lac::editor
