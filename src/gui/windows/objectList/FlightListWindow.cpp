#include "FlightListWindow.h"

#include <QKeyEvent>
#include <QPushButton>

#include "src/util/qString.h"

FlightListWindow::FlightListWindow (DbManager &manager, QWidget *parent):
	QMainWindow (parent),
	manager (manager)
{
	ui.setupUi(this);
	ui.buttonBox->button (QDialogButtonBox::Close)->setText (utf8 ("&Schließen"));

	QObject::connect (&manager, SIGNAL (stateChanged (DbManager::State)), this, SLOT (databaseStateChanged (DbManager::State)));
}

FlightListWindow::~FlightListWindow()
{

}

void FlightListWindow::show (DbManager &manager, QWidget *parent)
{
	// Create the window
	FlightListWindow *window = new FlightListWindow (manager, parent);
	window->setAttribute (Qt::WA_DeleteOnClose, true);

	// Show the window
	window->show ();
}

void FlightListWindow::on_actionClose_triggered ()
{
	close ();
}

void FlightListWindow::keyPressEvent (QKeyEvent *e)
{
//	std::cout << "FlightListWindow key " << e->key () << std::endl;

	// KeyEvents are accepted by default
	switch (e->key ())
	{
		case Qt::Key_Escape: ui.actionClose->trigger(); break;
		default: e->ignore (); break;
	}

	if (!e->isAccepted ()) QMainWindow::keyPressEvent (e);
}

void FlightListWindow::databaseStateChanged (DbManager::State state)
{
	if (state==DbManager::stateDisconnected)
		close ();
}

void FlightListWindow::on_actionSelectDate_triggered ()
{

}

void FlightListWindow::on_actionExport_triggered ()
{

}
