#include <lac/editor/EditorHighlighter.h>

namespace lac::editor
{
	EditorHighlighter::EditorHighlighter(QTextDocument* document)
		: QSyntaxHighlighter(document)
	{
	}

	void EditorHighlighter::useLuaRules()
	{
		auto addRule = [this](QString regex, QColor forecolor, bool isGlobal = true) {
			HighlightRule rule;
			rule.regex = QRegularExpression{regex};
			rule.foreground = forecolor;
			rule.isGlobal = isGlobal;
			rules.push_back(std::move(rule));
		};

		auto addMultilineRule = [this](QString startRegex, QString endRegex, QColor forecolor, bool isGlobal = true) {
			HighlightRule rule;
			rule.regex = QRegularExpression{startRegex};
			rule.end_regex = QRegularExpression{endRegex};
			rule.foreground = forecolor;
			rule.isGlobal = isGlobal;
			rules.push_back(std::move(rule));
		};

		// Binary, hexadecimal and decimal numbers
		addRule(R"~~(\b\d+\b|\b0[xX][0-9a-fA-F]+\b|\b0[b][01]+\b)~~", "#c3e88d");

		// Strings using simple quote
		addRule(R"~~(\'[^\']*\')~~", "#d69d85");

		// Strings using double quotes
		addRule(R"~~(\"[^\"]*\")~~", "#d69d85");

		// Strings using brackets
		addRule("[^-]\\[\\[.*\\]\\]", "#d69d85");

		// Multiline strings
		addMultilineRule("[^-]\\[\\[[^$]*", ".*\\]\\]", "#d69d85", false);

		// Lua keywords
		addRule("\\b(and|break|do|else|elseif|end|false|for|function|goto|if|"
				"in|local|nil|not|or|repeat|return|then|true|until|while)\\b",
				"#569cd6");

		// Metatables methods
		addRule("\\b(__add|__sub|__mul|__div|__mod|__pow|__unm|__idiv|__band|__bo|__bxor|"
				"__bnot|__shl|__shr|__concat|__len|__eq|__lt|__le|__index|__newindex|__call)\\b",
				"#569cd6");

		// Singleline comments
		addRule("--[^\\[].*$", "#57a64a", false);

		// Multiline comments
		addMultilineRule("--\\[\\[.*", ".*\\]\\]", "#87a64a", false);
	}

	void EditorHighlighter::highlightBlock(const QString& text)
	{
		setCurrentBlockState(-1);

		auto getFormat = [](const HighlightRule& rule) {
			QTextCharFormat format;
			format.setForeground(rule.foreground);
			format.setBackground(rule.background);
			return format;
		};

		// Apply highlighting rules
		for (int index = 0; index < static_cast<int>(rules.size()); ++index)
		{
			const auto& rule = rules[index];
			QRegularExpressionMatch match;

			bool hasEndRegex = !rule.end_regex.pattern().isEmpty();
			if (rule.isGlobal)
			{
				QRegularExpressionMatchIterator iter = rule.regex.globalMatch(text);
				if (iter.hasNext() && hasEndRegex && currentBlockState() == -1)
					setCurrentBlockState(index); // We will test the end later

				// It can have multiple lines
				while (iter.hasNext())
				{
					match = iter.next();
					setFormat(match.capturedStart(), match.capturedLength(), getFormat(rule));
				}
			}
			else
			{
				// Searches for only one match
				match = rule.regex.match(text);

				if (match.hasMatch())
				{
					setFormat(match.capturedStart(), match.capturedLength(), getFormat(rule));

					if (hasEndRegex && currentBlockState() == -1)
					{
						// Test if the rule end on this line
						QRegularExpression mulEnd{rule.end_regex};
						if (!mulEnd.match(text).hasMatch())
							setCurrentBlockState(index); // It continues
					}
				}
			}
		}

		// Close multiline rules
		if (previousBlockState() != -1)
		{
			const auto& rule = rules[previousBlockState()];
			QRegularExpressionMatch match = rule.end_regex.match(text);

			if (match.hasMatch())
			{
				// End the rule
				setFormat(match.capturedStart(), match.capturedLength(), getFormat(rule));
				setCurrentBlockState(-1);
			}
			else
			{
				// Continue the rule for the next line
				setFormat(0, text.length(), getFormat(rule));
				setCurrentBlockState(previousBlockState());
			}
		}
	}
} // namespace lac::editor
