#include "CsvExportDialog.h"

#include <iostream>

#include <QTextCodec>

#include "src/util/qString.h"

CsvExportDialog::CsvExportDialog (QWidget *parent):
	QDialog (parent)
{
	ui.setupUi (this);

	QList<int> mibs=QTextCodec::availableMibs ();
	foreach (int mib, mibs)
	{
		QTextCodec *codec=QTextCodec::codecForMib (mib);
		std::cout << QString::fromUtf8 (codec->name ()) << std::endl;
	}
}

CsvExportDialog::~CsvExportDialog ()
{

}
