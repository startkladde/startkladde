#include "FlightListWindow.h"

#include <QKeyEvent>
#include <QPushButton>

#include "src/model/Flight.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/util/qDate.h"
#include "src/gui/windows/input/DateInputDialog.h"
#include "src/model/objectList/MutableObjectList.h"
#include "src/model/flightList/FlightModel.h"
#include "src/model/objectList/ObjectListModel.h"

FlightListWindow::FlightListWindow (DbManager &manager, QWidget *parent):
	QMainWindow (parent),
	manager (manager)
{
	ui.setupUi(this);
	ui.buttonBox->button (QDialogButtonBox::Close)->setText (utf8 ("&Schließen"));

	QObject::connect (&manager, SIGNAL (stateChanged (DbManager::State)), this, SLOT (databaseStateChanged (DbManager::State)));

	flightList=new MutableObjectList<Flight> ();
	flightModel=new FlightModel (manager.getCache ());
	flightListModel=new ObjectListModel<Flight> (flightList, true, flightModel, true, this);

	// FIXME no color?
	ui.table->setModel (flightListModel); // FIXME sort proxy
	ui.table->setAutoResizeRows (true);
}

FlightListWindow::~FlightListWindow()
{
	ui.table->setModel (NULL);
	// FIXME check
	// flightListModel deleted by parent
	// flightModel and flightList deleted by listModel (owned)
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
	// FIXME test aborting

	// Get the flights from the database
	QList<Flight> flights=manager.getFlights (first, last, this);

	// Store the (new) first and last date
	currentFirst=first;
	currentLast=last;

	// Create and set the descriptive text: "1.1.2011 bis 31.12.2011: 123 Flüge"
	int numFlights=flights.size ();
	QString dateText=toString (currentFirst, currentLast, " bis ");
	QString numFlightsText=countText (numFlights, "Flug", utf8 ("Flüge"), utf8 ("keine Flüge"));
	ui.captionLabel->setText (QString ("%1: %2").arg (dateText).arg (numFlightsText));

	flightList->replaceList (flights);
	ui.table->resizeColumnsToContents ();

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
