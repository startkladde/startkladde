#include "ExportDatabaseDialog.h"

#include <iostream>

#include "src/util/qString.h"

// TODO: the DateEdit widgets' text is clipped because the calenar popup button
// is too wide

// TODO:
//   * show date for "today" and "yesterday"
//   * set default date for single (today) and range (first of year to today)

ExportDatabaseDialog::ExportDatabaseDialog (QWidget *parent):
	QDialog (parent)
{
	ui.setupUi (this);

	QString displayFormat=ui.singleDateInput->displayFormat ();

	// Hack to replace yy with yyyy, but leave yyyy intact
	displayFormat.replace ("yy", "yyyy").replace ("yyyyyyyy", "yyyy");

	ui.singleDateInput->setDisplayFormat (displayFormat);
	ui.firstDateInput->setDisplayFormat (displayFormat);
	ui.lastDateInput->setDisplayFormat (displayFormat);
}

ExportDatabaseDialog::~ExportDatabaseDialog()
{
}
