#include "FlightListWindow.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QtAlgorithms>

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

	ui.table->setModel (proxyModel);
	ui.table->setAutoResizeRows (true);
}

FlightListWindow::~FlightListWindow ()
{
	// flightListModel is deleted by this class, which is its Qt parent.
	// flightModel and flightList are deleted by flightListModel, which owns
	// them.
}

void FlightListWindow::show (DbManager &manager, QWidget *parent)
{
	// Get the date range
	QDate first, last;
	if (DateInputDialog::editRange (&first, &last, "Datum eingeben", "Datum eingeben:", parent))
	{
		// Create the window
		FlightListWindow *window = new FlightListWindow (manager, parent);
		window->setAttribute (Qt::WA_DeleteOnClose, true);

		// Get the flights from the database
		window->setDateRange (first, last);
		window->show ();
	}
}

void FlightListWindow::fetchFlights ()
{
	// FIXME test aborting (both on opening and refreshing/changing date)

	// Get the flights from the database
	QList<Flight> flights=manager.getFlights (currentFirst, currentLast, this);

	// Sort the flights (by effective date)
	// TODO: hide the sort indicator. The functionality is already in
	// MainWindow, should probably be in SkTableView.
	qSort (flights);

	// Create and set the descriptive text: "1.1.2011 bis 31.12.2011: 123 Flüge"
	int numFlights=flights.size ();
	QString dateText=toString (currentFirst, currentLast, " bis ");
	QString numFlightsText=countText (numFlights, "Flug", utf8 ("Flüge"), utf8 ("keine Flüge"));
	ui.captionLabel->setText (QString ("%1: %2").arg (dateText).arg (numFlightsText));

	flightList->replaceList (flights);
	ui.table->resizeColumnsToContents ();
}

void FlightListWindow::setDateRange (const QDate &first, const QDate &last)
{
	// Store the (new) first and last date, reversing the range if necessary
	if (first<=last)
	{
		currentFirst=first;
		currentLast=last;
	}
	else
	{
		currentFirst=last;
		currentLast=first;
	}

	fetchFlights ();
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
