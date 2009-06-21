#include "SkTableItem.h"


// ******************
// ** Construction **
// ******************

SkTableItem::SkTableItem ()
	:QTableWidgetItem ()
{
	init ();
}

SkTableItem::SkTableItem (const QString &text, QColor bg)
	:QTableWidgetItem (text)
{
	init (bg);
}

SkTableItem::SkTableItem (const string &text, QColor bg)
	:QTableWidgetItem (std2q (text))
{
	init (bg);
}

SkTableItem::SkTableItem (const char *text, QColor bg)
	:QTableWidgetItem ((QString)text)
{
	init (bg);
}

void SkTableItem::init ()
{
	data_id=0;
	data=NULL;
	setFlags (flags () &~ Qt::ItemIsEditable);
}

void SkTableItem::init (QColor bg)
{
	init ();
	setBackgroundColor (bg);
}
