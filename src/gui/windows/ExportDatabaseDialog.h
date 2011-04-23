#ifndef EXPORTDATABASEDIALOG_H
#define EXPORTDATABASEDIALOG_H

#include <QtGui/QDialog>
#include "ui_ExportDatabaseDialog.h"

class ExportDatabaseDialog: public QDialog
{
		Q_OBJECT

	public:
		ExportDatabaseDialog (QWidget *parent = 0);
		~ExportDatabaseDialog ();

	private:
		Ui::ExportDatabaseDialogClass ui;
};

#endif
