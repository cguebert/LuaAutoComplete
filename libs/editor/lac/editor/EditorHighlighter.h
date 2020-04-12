#pragma once

#include <lac/editor_api.h>

#include <QBrush>
#include <QRegularExpression>
#include <QSyntaxHighlighter>

#include <vector>

class QTextDocument;

namespace lac::editor
{
	struct HighlightRule
	{
		QRegularExpression regex, end_regex;
		QBrush background, foreground;
		bool isGlobal = true;
	};

	class EDITOR_API EditorHighlighter : public QSyntaxHighlighter
	{
		Q_OBJECT
	public:
		EditorHighlighter(QTextDocument* document);

		void useLuaRules();

		std::vector<HighlightRule> rules;

	protected:
		void highlightBlock(const QString& text) override;
	};
} // namespace lac::editor
