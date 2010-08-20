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
class TableButton;
class QMouseEvent;

/**
 * Adds some functionality to QTableView:
 *   - on double click on empty (non-cell) space, doubleClicked is emitted with
 *     an invalid index
 *   - ...
 */
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
		value_accessor (bool, ColoredSelectionEnabled, coloredSelectionEnabled);

		// Settings
		void readColumnWidths (QSettings &settings, const ColumnInfo &columnInfo);
		void writeColumnWidths (QSettings &settings, const ColumnInfo &columnInfo);

		bool cellVisible (const QModelIndex &index);
		QWidget *findVisibleWidget (const QModelIndexList &indexes);
		QPersistentModelIndex findButton (TableButton *button);
		bool focusWidgetAt (const QModelIndex &index);


	public slots:
		virtual void reset ();
		virtual void layoutChanged ();

	signals:
		void buttonClicked (QPersistentModelIndex index);
		void emptySpaceDoubleClicked ();

	protected slots:
//		virtual void rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		virtual void rowsInserted (const QModelIndex &parent, int start, int end);
		virtual void dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		virtual void currentChanged (const QModelIndex &current, const QModelIndex &previous);
		virtual void selectionChanged (const QItemSelection &selected, const QItemSelection &deselected);

	protected:
		void updateButtons (int row);
		void updateWidgetFocus (const QModelIndexList &indexes);
		virtual void mouseDoubleClickEvent (QMouseEvent *event);
		virtual void keyPressEvent (QKeyEvent *e);
		void scrollLeft ();
		void scrollRight ();
		void updateSelectionColors ();

		bool autoResizeRows;

	private:
		// HACK: updateButtons calls setIndexWidget which calls dataChanged (as
		// of Qt 4.5 - 4.3 didn't) which usually calls updateButtons. This is
		// to avoid this recursion. This may not be the best solution for this
		// problem.
		bool settingButtons;
		bool coloredSelectionEnabled;
};

#endif
