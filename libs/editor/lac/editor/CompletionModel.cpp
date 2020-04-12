#include <lac/editor/CompletionModel.h>

namespace lac::editor
{
	CompletionModel::CompletionModel(QObject* parent)
		: QAbstractListModel(parent)
	{
	}

	int CompletionModel::rowCount(const QModelIndex& parent) const
	{
		if (parent.isValid())
			return 0;

		return static_cast<int>(m_completions.size());
	}

	QVariant CompletionModel::data(const QModelIndex& index, int role) const
	{
		if (!index.isValid())
			return {};

		const auto row = index.row();
		if (role == Qt::EditRole || role == Qt::DisplayRole)
			return m_completions[row];

		return {};
	}

	void CompletionModel::setCompletionList(QStringList list)
	{
		beginResetModel();
		m_completions = list;
		endResetModel();
	}
} // namespace lac::editor
