#ifndef SKTABLEWIDGET_H_
#define SKTABLEWIDGET_H_

#include <QTableWidget>
#include <QPersistentModelIndex>

class QAbstractItemModel;
class QWidget;
class QSettings;

class ColumnInfo;

/**
 * A table view which does not read the data from the model (the effective
 * model) repeatedly but instead caches the data. It is based on a QTableWidget
 * which uses an internal model for storing the data.
 *
 * Note that this is NOT a generic table view. The effective model must meet the
 * following assumptions:
 *   - the number of columns does not change
 *   - the header data for the columns does not change
 *   - layout changes only effect whole rows
 * Also:
 *   - the header data for the rows is ignored
 *
 * The following features are also added:
 *   - interpretation of button roles
 *   - ...
 *
 * You should never modify the data of the table view directly (for example,
 * by using the setItem method).
 */
class SkTableWidget: public QTableWidget
{
		Q_OBJECT

	public:
		SkTableWidget (QWidget *parent=NULL);
		SkTableWidget (int rows, int columns, QWidget *parent=NULL);
		~SkTableWidget ();

		QAbstractItemModel *getEffectiveModel ();
		void setEffectiveModel (QAbstractItemModel *effectiveModel);

		void readColumnWidths (QSettings &settings, const ColumnInfo &columnInfo);

	signals:
		void buttonClicked (QPersistentModelIndex index);

	protected:
		void updateRowCount ();
		void updateColumnCount ();
		void clearTable ();
		void removeAllRows ();
		void setCell (int row, int column);
		void setRow (int row);
		void setHeaders ();

	protected slots:
		void model_modelReset ();

		void model_rowsInserted (const QModelIndex &parent, int start, int end);
		void model_rowsRemoved (const QModelIndex &parent, int start, int end);

		void model_dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);

		void model_layoutAboutToBeChanged ();
		void model_layoutChanged ();

		// Not used - columns and headers of the model may not change
//		void columnsInserted (const QModelIndex &parent, int start, int end);
//		void columnsRemoved (const QModelIndex &parent, int start, int end);
//		void headerDataChanged (Qt::Orientation orientation, int first, int last);

	private:
		QAbstractItemModel *effectiveModel;
		// Temporarily stores the selected index on layout changes
		QPersistentModelIndex selectedIndex;
		QList<QPersistentModelIndex> indexes;
};

#endif
