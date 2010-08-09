/*
 * SkTableView.h
 *
 *  Created on: Aug 29, 2009
 *      Author: Martin Herrmann
 */

#ifndef SKTABLEVIEW_H_
#define SKTABLEVIEW_H_

#include <QTableView>

#include "src/accessor.h"

class ColumnInfo;
class QSettings;

class SkTableView: public QTableView
{
	Q_OBJECT

	public:
		// Construction
		SkTableView (QWidget *parent=NULL);
		virtual ~SkTableView ();

		// Property access
		virtual void setModel (QAbstractItemModel *model);

		value_accessor (bool, AutoResizeRows, autoResizeRows);

		// Settings
		void readColumnWidths (QSettings &settings, const ColumnInfo &columnInfo);
		void writeColumnWidths (QSettings &settings, const ColumnInfo &columnInfo);

	public slots:
		virtual void reset ();
		virtual void layoutChanged ();

	signals:
		void buttonClicked (QPersistentModelIndex index);

	protected slots:
//		virtual void rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		virtual void rowsInserted (const QModelIndex &parent, int start, int end);
		virtual void dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
//		virtual void currentChanged (const QModelIndex &current, const QModelIndex &previous);
//		virtual void selectionChanged (const QItemSelection &selected, const QItemSelection &deselected);

	protected:
		void updateButtons (int row);
		void keyPressEvent (QKeyEvent *e);

		bool autoResizeRows;

	private:
		// HACK: updateButtons calls setIndexWidget which calls dataChanged (as
		// of Qt 4.5 - 4.3 didn't) which usually calls updateButtons. This is
		// to avoid this recursion. This may not be the best solution for this
		// problem.
		bool settingButtons;
};

#endif
