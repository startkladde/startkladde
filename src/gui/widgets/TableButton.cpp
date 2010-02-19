/*
 * TableButton.cpp
 *
 *  Created on: Aug 30, 2009
 *      Author: mherrman
 */

#include "TableButton.h"

#include <iostream>

TableButton::TableButton (QPersistentModelIndex index, QWidget *parent):
	QPushButton (parent),
	index (index)
{
	init ();
}

TableButton::TableButton (QPersistentModelIndex index, const QString &text, QWidget *parent):
	QPushButton (text, parent),
	index (index)
{
	init ();
}

TableButton::TableButton (QPersistentModelIndex index, const QIcon &icon, const QString &text, QWidget *parent):
	QPushButton (icon, text, parent),
	index (index)
{
	init ();
}

TableButton::~TableButton ()
{
//	std::cout << "-button" << std::endl;
}

void TableButton::init ()
{
//	std::cout << "+button" << std::endl;
	QObject::connect (this, SIGNAL (clicked ()), this, SLOT (clickedSlot ()));
//	setText (QString ("[%1, %2]").arg (index.row  ()).arg (index.column ()));
}

void TableButton::clickedSlot ()
{
//	setText (QString ("{%1, %2}").arg (index.row  ()).arg (index.column ()));
	emit clicked (index);
}

QSize TableButton::sizeHint () const
{
	QSize size=QPushButton::sizeHint ();
	size.setHeight (0);
	return size;
}
