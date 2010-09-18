#ifndef SKTABLEWIDGET_H_
#define SKTABLEWIDGET_H_

#include <QTableWidget>

class QAbstractItemModel;
class QWidget;
class QSettings;

class ColumnInfo;

/**
 * Adds some functionality to QTableWidget:
 *   - SkTableWidget cat get its data from a QAbstractItemModel. Other than
 *     SkTableView, which is usually used for this task, the data is requested
 *     from the model only when the data changes or if explicitly requested.
 *     This might improve the performance significantly, especially where table
 *     buttons are involved.
 *     Note that the model is called dataModel rather than model to avoid
 *     interference with the actual model used by the table.
 */
class SkTableWidget: public QTableWidget
{
		Q_OBJECT

	public:
		SkTableWidget (QWidget *parent=NULL);
		SkTableWidget (int rows, int columns, QWidget *parent=NULL);
		~SkTableWidget ();

		QAbstractItemModel *getDataModel ();
		void setDataModel (QAbstractItemModel *dataModel);

		void readColumnWidths (QSettings &settings, const ColumnInfo &columnInfo);

	protected:
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

		void model_layoutChanged ();

//		void columnsInserted (const QModelIndex &parent, int start, int end);
//		void columnsRemoved (const QModelIndex &parent, int start, int end);
//		void headerDataChanged (Qt::Orientation orientation, int first, int last);
//		void layoutAboutToBeChanged ();

	private:
		QAbstractItemModel *dataModel;
};

#endif
