#include <editor/CompletionModel.h>
#include <editor/LuaEditor.h>

#include <lac/completion/type_at_pos.h>

#include <QAbstractItemView>
#include <QCompleter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTimer>
#include <QToolTip>

namespace lac::editor
{
	LuaEditor::LuaEditor(QWidget* parent)
		: QPlainTextEdit(parent)
		, m_completer(new QCompleter)
		, m_completionModel(new CompletionModel)
	{
		m_completer->setWidget(this);
		m_completer->setCompletionMode(QCompleter::PopupCompletion);
		m_completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
		m_completer->setFilterMode(Qt::MatchFlag::MatchContains);
		m_completer->setModel(m_completionModel);

		auto popup = m_completer->popup();
		popup->setFrameShape(QFrame::NoFrame);
		popup->setSelectionMode(QAbstractItemView::SingleSelection);
		popup->setSelectionBehavior(QAbstractItemView::SelectItems);
		popup->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

		setDesign({});

		// This is an ugly way to test the completion
		auto timer = new QTimer(this);
		connect(timer, &QTimer::timeout, this, &LuaEditor::updateProgram);
		timer->start(1000);

		connect(this, &QPlainTextEdit::textChanged, this, [this] {
			m_textChanged = true;
		});

		connect(m_completer, QOverload<const QString&>::of(&QCompleter::activated), this, &LuaEditor::completeWord);
	}

	void LuaEditor::setDesign(const EditorDesign& design)
	{
		auto popup = m_completer->popup();

		// Fonts
		auto editorFont = design.editor_font;
		editorFont.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);
		setFont(editorFont);

		auto popupFont = design.completion_font;
		popupFont.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);
		popup->setFont(popupFont);

		// For everything else, use stylesheets
		setStyleSheet(QString("QPlainTextEdit {"
							  "   border-top: %2px solid %1;"
							  "   border-right: %3px solid %1;"
							  "   border-bottom: %4px solid %1;"
							  "   border-left: %5px solid %1;"
							  "   background-color: %6;"
							  "   color: %7;"
							  "}")
						  .arg(design.editor_border_color.name())
						  .arg(design.editor_border.top())
						  .arg(design.editor_border.right())
						  .arg(design.editor_border.bottom())
						  .arg(design.editor_border.left())
						  .arg(design.editor_back_color.name())
						  .arg(design.editor_text_color.name()));

		popup->setStyleSheet(QString("QListView {"
									 "   border-top: %2px solid %1;"
									 "   border-left: %3px solid %1;"
									 "   border-right: %4px solid %1;"
									 "   border-bottom: %5px solid %1;"
									 "   background-color: %6;"
									 "   color: %7;"
									 "   outline: none;"
									 "}"
									 "QListView::item:selected {"
									 "   padding: -1px;"
									 "   border: 1px solid %8;"
									 "   background-color: %9;"
									 "   color: %10;"
									 "}"
									 "QListView::item:!selected:hover {"
									 "   background: transparent;"
									 "}")
								 .arg(design.completion_border_color.name())
								 .arg(design.completion_border.top())
								 .arg(design.completion_border.right())
								 .arg(design.completion_border.bottom())
								 .arg(design.completion_border.left())
								 .arg(design.completion_back_color.name())
								 .arg(design.completion_text_color.name())
								 .arg(design.completion_selection_border_color.name())
								 .arg(design.completion_selection_back_color.name())
								 .arg(design.completion_selection_text_color.name()));
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

		if (handleTabs(event))
			return;

		QPlainTextEdit::keyPressEvent(event);

		handleCompletion(event);
	}

	bool LuaEditor::handleTabs(QKeyEvent* event)
	{
		static QChar par(0x2029); // Qt uses paragraph separators

		// Adds tabs in front of the selected block(s)
		if (event->key() == Qt::Key_Tab && !textCursor().selectedText().isEmpty())
		{
			// Retrieves the amount of lines within the selected text
			QTextCursor cursor = textCursor();
			QString selected = cursor.selectedText();
			qint32 nbLines = selected.count(par) + 1;

			// If only one line is selected, replace the selection by a tab
			if (nbLines == 1)
			{
				cursor.beginEditBlock();
				cursor.removeSelectedText();
				cursor.insertText("\t");
				cursor.endEditBlock();
				return true;
			}

			// Retrieves the selection and the cursor positions
			int start = cursor.selectionStart(), end = cursor.selectionEnd();
			int pos = cursor.position();
			cursor.clearSelection();
			cursor.setPosition(start);
			cursor.beginEditBlock();

			// Inserts tabs for each selected line
			for (int i = 0; i < nbLines; ++i)
			{
				cursor.movePosition(QTextCursor::StartOfLine);
				cursor.insertText("\t");
				cursor.movePosition(QTextCursor::Down);
			}

			// Selects all the text
			if (pos == start)
			{
				cursor.setPosition(end + nbLines);
				cursor.setPosition(start + 1, QTextCursor::KeepAnchor);
			}
			else
			{
				cursor.setPosition(start + 1);
				cursor.setPosition(end + nbLines, QTextCursor::KeepAnchor);
			}

			cursor.endEditBlock();
			setTextCursor(cursor);
			return true;
		}

		// Removes tabs in front of selected block(s)
		if (event->key() == Qt::Key_Backtab && !textCursor().selectedText().isEmpty())
		{
			// Retrieves the amount of lines within the selected text
			QTextCursor cursor = textCursor();
			QString selected = cursor.selectedText();
			qint32 nbLines = selected.count(par) + 1;

			// Does not do anything if only one line is selected
			if (nbLines == 1)
				return true;

			// Retrieves the selection and the cursor positions
			int start = cursor.selectionStart(), end = cursor.selectionEnd();
			bool cursorAtStart = (cursor.position() == start);
			cursor.clearSelection();
			cursor.setPosition(start);
			cursor.movePosition(QTextCursor::StartOfLine);
			int minStart = cursor.position();
			cursor.beginEditBlock();

			// Removes a tab from each line
			for (int i = 0; i < nbLines; ++i)
			{
				cursor.movePosition(QTextCursor::StartOfLine);
				cursor.movePosition(QTextCursor::NextWord);
				cursor.movePosition(QTextCursor::StartOfWord);

				int prev = cursor.position();

				QString text = cursor.block().text();
				int pos = cursor.positionInBlock();
				if (!pos)
					continue;
				--pos;

				// The first character of the line is not a whitespace, so the cursor was set to the next word
				if (pos && !text.at(pos - 1).isSpace())
					continue;

				if (text.at(pos) == '\t')
					cursor.deletePreviousChar();
				else if (text.at(pos) == ' ')
				{
					for (int i = 0; i < 4 && pos >= 0; ++i, --pos)
					{
						if (text.at(pos) != ' ')
							break;
						cursor.deletePreviousChar();
					}
				}

				int delta = prev - cursor.position();
				end -= delta;
				if (!i)
					start = std::max(minStart, start - delta);
				cursor.movePosition(QTextCursor::Down);
			}

			// Selects all the text
			if (cursorAtStart)
			{
				cursor.setPosition(end);
				cursor.setPosition(start, QTextCursor::KeepAnchor);
			}
			else
			{
				cursor.setPosition(start);
				cursor.setPosition(end, QTextCursor::KeepAnchor);
			}

			cursor.endEditBlock();
			setTextCursor(cursor);
			return true;
		}

		// Insert a tab
		if (event->key() == Qt::Key_Tab)
		{
			QTextCursor cursor = textCursor();
			cursor.insertText("\t");
			setTextCursor(cursor);
			return true;
		}
		// Remove whitespace before the cursor
		else if (event->key() == Qt::Key_Backtab)
		{
			QTextCursor cursor = textCursor();
			QString text = cursor.block().text();
			int pos = cursor.positionInBlock();
			if (!pos)
				return true;
			--pos;
			if (text.at(pos) == '\t')
				cursor.deletePreviousChar();
			else if (text.at(pos) == ' ')
			{
				cursor.beginEditBlock();
				for (int i = 0; i < 4 && pos; ++i, --pos)
				{
					if (text.at(pos) != ' ')
						break;
					cursor.deletePreviousChar();
				}
				cursor.endEditBlock();
			}
			setTextCursor(cursor);
			return true;
		}

		return false;
	}

	void LuaEditor::handleCompletion(QKeyEvent* event)
	{
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
