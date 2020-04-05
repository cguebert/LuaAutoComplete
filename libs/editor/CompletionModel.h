#pragma once

#include <QAbstractListModel>

namespace lac::editor
{
	class CompletionModel : public QAbstractListModel
	{
	public:
		explicit CompletionModel(QObject* parent = nullptr);

		int rowCount(const QModelIndex& parent = {}) const override;

		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

		void setCompletionList(QStringList list);

	private:
		QStringList m_completions;
	};

} // namespace lac::editor
