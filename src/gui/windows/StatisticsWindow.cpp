#include "StatisticsWindow.h"

#include <QPushButton>

#include "src/util/qString.h"

StatisticsWindow::StatisticsWindow (QAbstractTableModel *model, bool modelOwned, QWidget *parent):
	QDialog(parent),
	model (model), modelOwned (modelOwned)
{
	ui.setupUi(this);
	ui.buttonBox->button (QDialogButtonBox::Close)->setText (utf8 ("&Schließen"));

	ui.table->setModel (model);

	ui.table->resizeColumnsToContents ();
	ui.table->resizeRowsToContents ();
}

StatisticsWindow::~StatisticsWindow()
{
	if (modelOwned)
		delete model;
}

void StatisticsWindow::display (QAbstractTableModel *model, bool modelOwned, QString title, QWidget *parent)
{
	StatisticsWindow *window=new StatisticsWindow (model, modelOwned, parent);
	window->setWindowTitle (title);
	window->setAttribute (Qt::WA_DeleteOnClose, true);
	window->show ();
}
