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

    	int getSelectedMib ();
    	QTextCodec *getSelectedCodec ();
    	QString getSeparator ();

    private slots:
    	void on_buttonBox_accepted ();


	private:
    	void addCodecEntry (int mib);

    	Ui::CsvExportDialogClass ui;

    	int selectedMib;
};

#endif
