#include <editor/CompletionModel.h>
#include <editor/LuaEditor.h>

#include <lac/completion/type_at_pos.h>

#include <QAbstractItemView>
#include <QCompleter>
#include <QScrollBar>
#include <QTimer>
#include <QToolTip>

namespace lac::editor
{
	LuaEditor::LuaEditor(QWidget* parent)
		: QPlainTextEdit(parent)
		, m_completer(new QCompleter)
		, m_completionModel(new CompletionModel)
	{
		// Specifies the default font for the code editor
		QFont editorFont("Monospace");
		editorFont.setPointSize(10);
		editorFont.setKerning(true);
		editorFont.setStyleHint(QFont::TypeWriter);
		editorFont.setStyleStrategy(QFont::PreferAntialias);
		setFont(editorFont);

		m_completer->setWidget(this);
		m_completer->setCompletionMode(QCompleter::PopupCompletion);
		m_completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
		m_completer->setFilterMode(Qt::MatchFlag::MatchContains);
		m_completer->setModel(m_completionModel);

		// This is an ugly way to test the completion
		auto timer = new QTimer(this);
		connect(timer, &QTimer::timeout, this, &LuaEditor::updateProgram);
		timer->start(1000);

		connect(this, &QPlainTextEdit::textChanged, this, [this] {
			m_textChanged = true;
		});

		connect(m_completer, QOverload<const QString&>::of(&QCompleter::activated), this, &LuaEditor::completeWord);
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

	void LuaEditor::keyPressEvent(QKeyEvent* event)
	{
		// Ignore some keys while the completion popup is visible
		if (m_completer->popup()->isVisible())
		{
			switch (event->key())
			{
			case Qt::Key_Tab:
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_Backtab:
				event->ignore();
				return;
			default:
				break;
			}
		}

		QPlainTextEdit::keyPressEvent(event);

		// Current text under the cursor
		auto cursor = textCursor();
		cursor.select(QTextCursor::WordUnderCursor);
		auto prefix = cursor.selectedText().trimmed();

		// Text on the left of the cursor
		cursor = textCursor();
		cursor.movePosition(QTextCursor::Left);
		cursor.select(QTextCursor::WordUnderCursor);
		const auto leftText = cursor.selectedText();

		auto isDot = [](const QString& str) {
			return str == "." || str == ":";
		};

		// Close the popup if no text under the cursor and backspace was pressed
		if (event->key() == Qt::Key_Backspace 
			&& prefix.isEmpty() 
			&& !isDot(leftText)) // But not if there is a dot on the left of the cursor
		{
			m_completer->popup()->hide();
			return;
		}

		if (isDot(prefix))
			prefix = "";

		// Ignore modifier key presses
		if ((event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) && event->text().isEmpty())
			return;

		if (m_completer->popup()->isVisible())
		{
			m_completer->setCompletionPrefix(prefix);
			
			if (!m_completer->completionCount()) // No completion
				m_completer->popup()->hide();
			else // Always select the first one so we can press enter to use it
				m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
			return;
		}

		bool refreshed = false;
		auto refreshCompletion = [this, prefix, &refreshed] {
			if (refreshed) // Ensure we only do it once in this function
				return;

			m_textChanged = true;
			updateProgram();
			m_completer->setCompletionPrefix(prefix);
			refreshed = true;
		};

		// When using Ctrl + space, try to complete the word without showing the popup
		bool ctrlSpace = event->key() == Qt::Key_Space
						 && event->modifiers() & Qt::ControlModifier;
		if (ctrlSpace)
		{
			refreshCompletion();
			if (m_completer->completionCount() == 1)
			{
				completeWord(m_completer->currentCompletion());
				return;
			}
		}

		auto askPopup = [ctrlSpace](QKeyEvent* event) {
			return event->key() == Qt::Key_Colon     // ':' -> member method
				   || event->key() == Qt::Key_Period // '.' -> member variable
				   || (ctrlSpace);                   // Ctrl + space
		};

		// Show the completion popup
		if (!m_completer->popup()->isVisible() && askPopup(event))
		{
			refreshCompletion();

			QRect rect = cursorRect();
			rect.moveTo(rect.x() - fontMetrics().horizontalAdvance(prefix), rect.y() + 4);
			rect.setWidth(m_completer->popup()->sizeHintForColumn(0) + m_completer->popup()->verticalScrollBar()->sizeHint().width());
			m_completer->complete(rect);
			m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
		}
	}

	void LuaEditor::completeWord(const QString& word)
	{
		auto cursor = textCursor();
		cursor.movePosition(QTextCursor::Left);

		// Test if there is only a '.' or ':' character
		cursor.select(QTextCursor::WordUnderCursor);
		const auto selection = cursor.selectedText();
		if (selection != "." && selection != ':')
		{
			cursor.movePosition(QTextCursor::StartOfWord);
			cursor.select(QTextCursor::WordUnderCursor);
			cursor.removeSelectedText();
		}

		cursor.clearSelection();
		cursor.insertText(word);
		setTextCursor(cursor);
	}

	void LuaEditor::updateProgram()
	{
		if (!m_textChanged)
			return;

		const auto text = document()->toPlainText().toStdString();
		const auto pos = textCursor().position() - 1;
		m_programCompletion.updateProgram(text, pos);
		m_textChanged = false;

		if (!m_completer->popup()->isVisible())
		{
			const auto elements = m_programCompletion.getAutoCompletionList(text, pos);
			QStringList list;
			for (const auto it : elements)
				list.push_back(QString::fromStdString(it.first));
			m_completionModel->setCompletionList(list);
		}
	}
} // namespace lac::editor
