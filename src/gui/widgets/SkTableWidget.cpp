/*
 * SkTableWidget.cpp
 *
 *  Created on: 17.09.2010
 *      Author: martin
 */

#include "SkTableWidget.h"

#include <iostream>
#include <cassert>

#include <QApplication>
#include <QAbstractItemModel>
#include <QSettings>
#include <QHeaderView>

#include "src/itemDataRoles.h"
#include "src/gui/widgets/TableButton.h"
#include "src/model/objectList/ColumnInfo.h"

SkTableWidget::SkTableWidget (QWidget *parent):
	QTableWidget (parent)
{

}

SkTableWidget::SkTableWidget (int rows, int columns, QWidget *parent):
	QTableWidget (rows, columns, parent)
{

}

SkTableWidget::~SkTableWidget ()
{
}

QAbstractItemModel *SkTableWidget::getDataModel ()
{
	return dataModel;
}

void SkTableWidget::setDataModel (QAbstractItemModel *dataModel)
{
	this->dataModel=dataModel;

#define reconnect(member) \
	disconnect (this, SLOT (model_ ## member)); \
	connect (dataModel, SIGNAL (member), this, SLOT (model_ ## member));

	reconnect (rowsInserted (const QModelIndex &, int, int));
	reconnect (rowsRemoved (const QModelIndex &, int, int));
	reconnect (dataChanged (const QModelIndex &, const QModelIndex &));
	reconnect (modelReset ());
	reconnect (layoutChanged ());

#undef reconnect

	model_modelReset ();
}

// FIXME almost same as in SkTableView
/**
 * Reads the column widths from the settings or uses defaults from a columnInfo
 *
 * The settings object has to be set to the correct section. The widths are
 * read from the value columnWidth_(name) where name is the column name from
 * columnInfo.
 *
 * If no width is stored in settings for a given column, the sample text from
 * columnInfo and the column title from the model are used to determine a
 * default width.
 *
 * @param settings the QSettings to read the widths from
 * @param columnInfo the ColumnInfo to read the default widths from
 */
void SkTableWidget::readColumnWidths (QSettings &settings, const ColumnInfo &columnInfo)
{
	// The column info set must have the same number of columns as the model of
	// this table.
	assert (columnInfo.columnCount ()==dataModel->columnCount ());

	for (int i=0; i<columnInfo.columnCount (); ++i)
	{
		// Determine the column name and the settings key
		QString columnName=columnInfo.columnName (i);
		QString key=QString ("columnWidth_%1").arg (columnName);

		// Determine the font metrics and the frame margin
		const QFont &font=horizontalHeader ()->font ();
		QFontMetrics metrics (font);
		QStyle *style=horizontalHeader ()->style ();
		if (!style) style=QApplication::style ();
		// Similar to QItemDelegate::textRectangle
		const int margin=style->pixelMetric (QStyle::PM_FocusFrameHMargin)+1;

		if (settings.contains (key))
		{
			// The settings contain a width for this column
			setColumnWidth (i, settings.value (key).toInt ());
		}
		else
		{
			// No width for this column in the settings. Determine the default.
			QString sampleText=columnInfo.sampleText (i);
			QString headerText=model ()->headerData (i, Qt::Horizontal).toString ();

			// The 2/4 were determined experimentally. Probably, some metric
			// should be used. For headerWidth, +2 is enough on Linux/Gnome,
			// but not on Windows XP.
			int sampleWidth=metrics.boundingRect (sampleText).width ()+2*margin+2;
			int headerWidth=metrics.boundingRect (headerText).width ()+2*margin+4;

			setColumnWidth (i, qMax (sampleWidth, headerWidth));
		}
	}
}

void SkTableWidget::clearTable ()
{
	// FIXME remove method?
	int rows=rowCount ();
	for (int i=0; i<rows; ++i)
		removeRow (0);

	int columns=columnCount ();
	for (int i=0; i<columns; ++i)
		removeColumn (0);
}


void SkTableWidget::setCell (int row, int column)
{
	QModelIndex index=dataModel->index (row, column);

	if (dataModel->data (index, isButtonRole).toBool ())
	{
		// *** It's a button

		QString text=dataModel->data (index, buttonTextRole).toString ();
		std::cout << "#" << std::flush;
		TableButton *button=new TableButton (index, text, this);

		setCellWidget (row, column, button);
	}
	else
	{
		// *** It's a regular cell

		QString text=dataModel->data (index, Qt::DisplayRole).toString ();
		QTableWidgetItem *item=new QTableWidgetItem (text);
		item->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		QBrush background=dataModel->data (index, Qt::BackgroundRole).value<QBrush> (); // TODO allow reusing
		item->setBackground (background);

		setItem (row, column, item);
	}
}

void SkTableWidget::setRow (int row)
{
	int columnCount=dataModel->columnCount ();
	for (int column=0; column<columnCount; ++column)
		setCell (row, column);
}

void SkTableWidget::setHeaders ()
{
	int ourColumnCount=columnCount ();
	int modelColumnCount=dataModel->columnCount ();

	// Too few columns? Add the difference.
	if (ourColumnCount<modelColumnCount)
		for (int column=ourColumnCount; column<modelColumnCount; ++column)
			insertColumn (column);

	// Too many columns? Remove the difference.
	if (ourColumnCount>modelColumnCount)
		for (int column=ourColumnCount-1; column>=modelColumnCount; --column)
			removeColumn (column);

	assert (columnCount ()==dataModel->columnCount ());

	// Column titles
	for (int column=0; column<modelColumnCount; ++column)
		setHorizontalHeaderItem (column, new QTableWidgetItem (
			dataModel->headerData (column, Qt::Horizontal).toString ()));
}

void SkTableWidget::removeAllRows ()
{
	int numRows=rowCount ();
	for (int i=numRows-1; i>=0; --i)
		removeRow (i);
}

// *******************
// ** Model changes **
// *******************

void SkTableWidget::model_modelReset ()
{
	std::cout << "model reset to " << dataModel->rowCount () << " rows" << std::endl;

	setHeaders ();

	removeAllRows ();

	int rowCount=dataModel->rowCount ();
	for (int row=0; row<rowCount; ++row)
	{
		insertRow (row);
		setRow (row);
	}

	// FIXME only if enabled, only changed ones?
	resizeRowsToContents ();
}

void SkTableWidget::model_rowsInserted (const QModelIndex &parent, int start, int end)
{
	assert (!parent.isValid ());
	std::cout << "model inserted rows " << start << " to " << end << std::endl;

	for (int i=start; i<=end; ++i)
	{
		insertRow (i);
		setRow (i);
	}
}

void SkTableWidget::model_rowsRemoved (const QModelIndex &parent, int start, int end)
{
	assert (!parent.isValid ());
	std::cout << "model removed rows " << start << " to " << end << std::endl;

	for (int i=end; i>=start; --i)
		removeRow (i);
}

void SkTableWidget::model_dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	std::cout << "model data changed " << topLeft.row () << ", " << topLeft.column ()
			<< " to " << bottomRight.row () << ", " << bottomRight.column () << std::endl;

	for (int row=topLeft.row (); row<=bottomRight.row (); ++row)
		for (int column=topLeft.column (); column<=bottomRight.column (); ++column)
			setCell (row, column);
}

void SkTableWidget::model_layoutChanged ()
{
	// Note that when the data changes (for example, after editing a flight),
	// the proxy model re-sorts the data and emits layoutChanged (even if the
	// layout is the same as before).

	// FIXME implement (how?)
	// Ugly solution: write the data to all columns (without deleting the columns)
	// and update the current selection (by using a persistent model index).
	std::cout << "oh no! the layout changed and we aren't handling it!" << std::endl;
	model_modelReset ();
}
