#include "SkTableView.h"

#include <QSettings>
#include <QFont>
#include <QFontMetrics>
#include <QHeaderView>
#include <QApplication>
#include <QStyle>
#include <QKeyEvent>

#include "src/gui/widgets/TableButton.h"
#include "src/itemDataRoles.h"
#include "src/concurrent/threadUtil.h" // Required for assert (isGuiThread ());
#include "src/model/objectList/ColumnInfo.h"

#include <iostream>
#include <cassert>

// TODO: verify that all buttons are deleted

SkTableView::SkTableView (QWidget *parent):
	QTableView (parent),
	autoResizeRows (false),
	settingButtons (false)
{
}

SkTableView::~SkTableView ()
{
}

void SkTableView::setModel (QAbstractItemModel *model)
{
	QTableView::setModel (model);

	QObject::disconnect (this, SLOT (layoutChanged ()));
	connect (model, SIGNAL (layoutChanged ()), this, SLOT (layoutChanged ()));
}

void SkTableView::layoutChanged ()
{
	// This happens when the SortFilterModel filter settings are changed
	if (autoResizeRows)
		resizeRowsToContents ();
}

void SkTableView::updateButtons (int row)
{
	// TODO: this should not happen, but when opening the person or plane
	// editor, it sometimes does (note that the models have no buttons in this
	// case).
//	assert (!settingButtons);

	if (settingButtons) return;

	QAbstractItemModel *m=model ();
	int columns=m->columnCount ();

	for (int column=0; column<columns; ++column)
	{
		QModelIndex index=m->index (row, column);

		if (m->data (index, isButtonRole).toBool ())
		{
			QString buttonText=m->data (index, buttonTextRole).toString ();
			TableButton *button=new TableButton (index, buttonText);
			QObject::connect (button, SIGNAL (clicked (QPersistentModelIndex)), this, SIGNAL (buttonClicked (QPersistentModelIndex)));


			// Avoid recursive calls, see above
			settingButtons=true;
			setIndexWidget (index, button);
			settingButtons=false;
		}
		else
		{
			// Avoid recursive calls, see above
			settingButtons=true;
			setIndexWidget (index, NULL);
			settingButtons=false;
		}
	}
}

//void SkTableView::rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
//{
//}

void SkTableView::rowsInserted (const QModelIndex &parent, int start, int end)
{
	QTableView::rowsInserted (parent, start, end);

	for (int i=start; i<=end; ++i)
		updateButtons (i);

	// If a row is inserted, the rows after that get renumbered. A row that
	// gets an index that did not exist before may be resized to the default
	// size (at least with 4.3.4). This may be a bug in Qt.
	// Workaround: resize all rows, not only the ones that were inserted.

	resizeRowsToContents ();

//	if (autoResizeRows)
//		for (int i=start; i<=end; ++i)
//			resizeRowToContents (i);
}

void SkTableView::dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	assert (isGuiThread ());

	QTableView::dataChanged (topLeft, bottomRight);

	if (settingButtons) return;

	for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
		updateButtons (i);

	if (autoResizeRows)
		for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
			resizeRowToContents (i);
}

void SkTableView::reset ()
{
	QTableView::reset ();

	// Set up the buttons
	int rows=model ()->rowCount ();
	for (int row=0; row<rows; ++row)
		updateButtons (row);

	if (autoResizeRows)
		resizeRowsToContents ();
}

void SkTableView::readColumnWidths (QSettings &settings, const ColumnInfo &columnInfo)
{
	assert (columnInfo.columnCount ()==model ()->columnCount ());

	for (int i=0; i<columnInfo.columnCount (); ++i)
	{
		QString columnName=columnInfo.columnName (i);
		QString key=QString ("columnWidth_%1").arg (columnName);

		const QFont &font=horizontalHeader ()->font ();
		QFontMetrics metrics (font);
		QStyle *style=horizontalHeader ()->style ();
		if (!style) style=QApplication::style ();
		// Similar to QItemDelegate::textRectangle
		const int margin=style->pixelMetric (QStyle::PM_FocusFrameHMargin)+1;

		if (settings.contains (key))
			setColumnWidth (i, settings.value (key).toInt ());
		else
			setColumnWidth (i, metrics.boundingRect (columnInfo.sampleText (i)).width ()+2*margin+2);
	}
}

void SkTableView::writeColumnWidths (QSettings &settings, const ColumnInfo &columnInfo)
{
	assert (columnInfo.columnCount ()==model ()->columnCount ());

	for (int i=0; i<columnInfo.columnCount (); ++i)
	{
		QString columnName=columnInfo.columnName (i);
		QString key=QString ("columnWidth_%1").arg (columnName);
		int value=columnWidth (i);

		settings.setValue (key, value);
	}
}

void SkTableView::keyPressEvent (QKeyEvent *e)
{
	// Hack: it seems that as of Qt 4.6.2 (Ubuntu Lucid), QTableView consumes
	// the delete key, which is not passed to the parent widget (the containing
	// window). This only seems to happen with the delete key proper, not the
	// keypad delete key, even though both have a value of Qt::Key_Delete.
	// Ignore the delete key here to propagate it to the parent widget.
	switch (e->key ())
	{
		case Qt::Key_Delete: e->ignore (); break;
		default: QTableView::keyPressEvent (e);
	}
}
