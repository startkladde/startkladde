#include "FlightListWindow.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QSortFilterProxyModel>

#include "src/model/Flight.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/util/qDate.h"
#include "src/gui/windows/input/DateInputDialog.h"
#include "src/model/objectList/MutableObjectList.h"
#include "src/model/flightList/FlightModel.h"
#include "src/model/objectList/ObjectListModel.h"

/*
 * Improvements:
 *   - Instead of a refresh action, track flight changes, either through
 *     AutomaticEntityList or by explicitly receiving DbEvent signals. This
 *     requires applying the filter to each changed flight.
 *   - Receive DbEvent signals to update person/plane changes immediately. At
 *     the moment, such changes are only updated when the window is activated,
 *     because the list view rereads the data from the cache.
 */

FlightListWindow::FlightListWindow (DbManager &manager, QWidget *parent):
	QMainWindow (parent),
	manager (manager)
{
	ui.setupUi(this);
	ui.buttonBox->button (QDialogButtonBox::Close)->setText (utf8 ("&Schließen"));

	QObject::connect (&manager, SIGNAL (stateChanged (DbManager::State)), this, SLOT (databaseStateChanged (DbManager::State)));

	// Set up the flight list and model
	flightList=new MutableObjectList<Flight> ();
	flightModel=new FlightModel (manager.getCache ());
	flightModel->setColorEnabled (false);
	flightListModel=new ObjectListModel<Flight> (flightList, true, flightModel, true, this);

	// Set up the sorting proxy model
	proxyModel=new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (flightListModel);

	// FIXME better model, and allow sorting by time/date
	ui.table->setModel (proxyModel);
	ui.table->setAutoResizeRows (true);
}

FlightListWindow::~FlightListWindow()
{
	// FIXME verify:
	//   - flightListModel: ObjectListModel
	//   - flightModel: FlightModel
	//   - flightList: MutableObjectList
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

void FlightListWindow::fetchFlights ()
{
	// FIXME handle date range reversed
	// FIXME test aborting (both on opening and refreshing/changing date)

	// Get the flights from the database
	QList<Flight> flights=manager.getFlights (currentFirst, currentLast, this);

	// Create and set the descriptive text: "1.1.2011 bis 31.12.2011: 123 Flüge"
	int numFlights=flights.size ();
	QString dateText=toString (currentFirst, currentLast, " bis ");
	QString numFlightsText=countText (numFlights, "Flug", utf8 ("Flüge"), utf8 ("keine Flüge"));
	ui.captionLabel->setText (QString ("%1: %2").arg (dateText).arg (numFlightsText));

	flightList->replaceList (flights);
	ui.table->resizeColumnsToContents ();
}

bool FlightListWindow::setDateRange (const QDate &first, const QDate &last)
{
	// Store the (new) first and last date
	currentFirst=first;
	currentLast=last;

	fetchFlights ();

	return true;
}

void FlightListWindow::on_actionRefresh_triggered ()
{
	fetchFlights ();
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
