#ifndef CSVEXPORTDIALOG_H
#define CSVEXPORTDIALOG_H

#include <QtGui/QDialog>
#include "ui_CsvExportDialog.h"

class CsvExportDialog : public QDialog
{
    Q_OBJECT

	public:
    	CsvExportDialog (QWidget *parent=0);
    	~CsvExportDialog ();

	private:
    	Ui::CsvExportDialogClass ui;
};

#endif
