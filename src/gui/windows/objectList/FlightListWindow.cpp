#include "FlightListWindow.h"

#include <QKeyEvent>
#include <QPushButton>

#include "src/model/Flight.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/util/qDate.h"
#include "src/gui/windows/input/DateInputDialog.h"

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

	// Get the date range
	QDate first, last;
	if (DateInputDialog::editRange (&first, &last, "Datum eingeben", "Datum eingeben:", parent))
	{
		// Get the flights from the database
		if (window->setDateRange (first, last))
		{
			// Show the window
			window->show ();
		}
	}
}

bool FlightListWindow::setDateRange (const QDate &first, const QDate &last)
{
	// FIXME handle date range reversed
	// FIXME implement fetch and display
	// FIXME test aborting
	// FIXME sometimes crashes

	QList<Flight> flights=manager.getFlights (first, last, this);

	// FIXME remove repeated getFlights
	for (int i=0; i<32; ++i)
		flights=manager.getFlights (first, last, this);

	if (false) return false;

	currentFirst=first;
	currentLast=last;

	int numFlights=flights.size ();
	QString dateText=toString (currentFirst, currentLast, " bis ");
	QString numFlightsText=countText (numFlights, "Flug", utf8 ("Flüge"), utf8 ("keine Flüge"));
	ui.captionLabel->setText (QString ("%1: %2").arg (dateText).arg (numFlightsText));

	return true;
}

void FlightListWindow::on_actionClose_triggered ()
{
	close ();
}

void FlightListWindow::keyPressEvent (QKeyEvent *e)
{
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
	QDate newFirst=currentFirst;
	QDate newLast =currentLast ;

	if (DateInputDialog::editRange (&newFirst, &newLast, "Datum eingeben", "Datum eingeben:", this))
		setDateRange (newFirst, newLast);
}

void FlightListWindow::on_actionExport_triggered ()
{
	// FIXME implement
}
