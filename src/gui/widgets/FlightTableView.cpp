#include "src/gui/widgets/FlightTableView.h"

#include <QHeaderView>
#include <QModelIndex>

#include "src/model/flightList/FlightModel.h"
#include "src/model/flightList/FlightProxyList.h"
#include "src/model/flightList/FlightSortFilterProxyModel.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/gui/dialogs.h"
#include "src/gui/widgets/TableButton.h"
#include "src/logging/messages.h"
#include "src/gui/widgets/NotificationWidget.h"


// flightList
// proxyList       = FlightProxyList            (flightList)
// flightListModel = ObjectListModel            (proxyList)
// proxyModel      = FlightSortFilterProxyModel (flightListModel)

// ******************
// ** Construction **
// ******************

FlightTableView::FlightTableView (QWidget *parent):
	SkTableView (parent),
	_flightList (NULL), _proxyList (NULL), _flightModel (NULL),
	_flightListModel (NULL), _proxyModel (NULL)
{
	// FIXME test
	connect (horizontalHeader (), SIGNAL (sectionClicked (int)),
		this, SLOT (sortByColumn (int)));

	// FIXME test
	connect (
		this, SIGNAL (buttonClicked (QPersistentModelIndex)),
		this, SLOT (base_buttonClicked (QPersistentModelIndex))
		);
}

FlightTableView::~FlightTableView ()
{
}

void FlightTableView::setModel (EntityList<Flight> *flightList, DbManager &dbManager)
{
	// FIXME most of these should be set in the constructor, and just update
	// here (where applicable) - what is created here must be deleted. The
	// remove the model check at the beginning of all methods where possible.
	_flightList=flightList;

	_proxyList=new FlightProxyList (dbManager.getCache (), *_flightList, this); // TODO never deleted

	_flightModel = new FlightModel (dbManager.getCache ());
	_flightListModel = new ObjectListModel<Flight> (_proxyList, false, _flightModel, true, this);

	_proxyModel = new FlightSortFilterProxyModel (dbManager.getCache (), this);
	_proxyModel->setSourceModel (_flightListModel);

	_proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	_proxyModel->setDynamicSortFilter (true);


	SkTableView::setModel (_proxyModel);
}

void FlightTableView::setHideFinishedFlights (bool hideFinishedFlights)
{
	if (!_flightList)
		return;

	_proxyModel->setHideFinishedFlights (hideFinishedFlights);
}

void FlightTableView::setAlwaysShowExternalFlights (bool alwaysShowExternalFlights)
{
	if (!_flightList)
		return;

	_proxyModel->setAlwaysShowExternalFlights (alwaysShowExternalFlights);
}

void FlightTableView::setAlwaysShowErroneousFlights (bool alwaysShowErroneousFlights)
{
	if (!_flightList)
		return;

	_proxyModel->setAlwaysShowErroneousFlights (alwaysShowErroneousFlights);
}

void FlightTableView::setShowPreparedFlights (bool showPreparedFlights)
{
	if (!_flightList)
		return;

	_proxyModel->setShowPreparedFlights (showPreparedFlights);
}

void FlightTableView::setCustomSorting (bool customSorting)
{
	if (!_flightList)
		return;

	_proxyModel->setCustomSorting (customSorting);
}

FlightReference FlightTableView::getCurrentFlightReference ()
{
	if (!_flightList)
		return FlightReference::invalid;

	// Get the currently selected index from the table; it refers to the
	// proxy model
	QModelIndex proxyIndex = currentIndex ();

	// Map the index from the proxy model to the flight list model
	QModelIndex flightListModelIndex = _proxyModel->mapToSource (proxyIndex);

	// If there is not selection, return an invalid ID
	if (!flightListModelIndex.isValid ())
		return FlightReference::invalid;

	// Get the flight from the model
	const Flight &flight = _flightListModel->at (flightListModelIndex);

	return FlightReference (flight);
}

bool FlightTableView::selectFlight (const FlightReference &flightReference, int column)
{
	if (!_flightList)
		return false;

	if (!flightReference.isValid ())
		return false;

	// Find the flight or towflight with that ID in the flight proxy list
	// FIXME should also use FlightReference
	int proxyListIndex=_proxyList->modelIndexFor (flightReference.id (), flightReference.towflight ());
	if (proxyListIndex<0) return false;

	// Create the index in the flight list model
	QModelIndex flightListModelIndex=_flightListModel->index (proxyListIndex, column);
	if (!flightListModelIndex.isValid ()) return false;

	// Map the index from the flight list model to the proxy model
	QModelIndex proxyIndex=_proxyModel->mapFromSource (flightListModelIndex);
	if (!proxyIndex.isValid ()) return false;

	// Select it
	setCurrentIndex (proxyIndex);

	return true;
}

void FlightTableView::sortCustom ()
{
	if (!_flightList)
		return;

	// Use custom sorting
	_proxyModel->sortCustom ();

	// Show the sort status in the header view
	setSortingEnabled (false); // Make sure it is off
	horizontalHeader ()->setSortIndicatorShown (false);
}

void FlightTableView::sortByColumn (int column)
{
	// FIXME Sortieren nach Zeiten etc.: vorbereitete immer hinten
	if (!_flightList)
		return;

	// FIXME 3-way toggle: ascending/descending/custom
	// Determine the new sorting order: when custom sorting was in effect or the
	// sorting column changed, sort ascending; otherwise, toggle the sorting
	// order
	if (_proxyModel->getCustomSorting ())
		_sortOrder=Qt::AscendingOrder; // custom sorting was in effect
	else if (column!=_sortColumn)
		_sortOrder=Qt::AscendingOrder; // different column
	else if (_sortOrder==Qt::AscendingOrder)
		_sortOrder=Qt::DescendingOrder; // toggle ascending->descending
	else
		_sortOrder=Qt::AscendingOrder; // toggle any->ascending

	// Set the new sorting column
	_sortColumn=column;

	// Sort the proxy model
	_proxyModel->setCustomSorting (false);
	_proxyModel->sort (_sortColumn, _sortOrder);

	// Show the sort status in the header view
	QHeaderView *header=horizontalHeader ();
	header->setSortIndicatorShown (true);
	header->setSortIndicator (_sortColumn, _sortOrder);
}

void FlightTableView::readColumnWidths (QSettings &settings)
{
	if (!_flightList)
		return;

	SkTableView::readColumnWidths (settings, *_flightModel);
}

void FlightTableView::writeColumnWidths (QSettings &settings)
{
	if (!_flightList)
		return;

	SkTableView::writeColumnWidths (settings, *_flightModel);
}

void FlightTableView::interactiveJumpToTowflight ()
{
	if (!_flightList)
		return;

	// Get the currently selected index in the ObjectListModel
	QModelIndex index=_proxyModel->mapToSource (currentIndex ());

	if (!index.isValid ())
	{
		showWarning (tr ("No flight selected"), tr ("No flight is selected."), this);
		return;
	}

	// Get the towref from the FlightProxyList. The rows of the ObjectListModel
	// correspond to those of its source, the FlightProxyList.
	int towref=_proxyList->findTowref (index.row ());

	// TODO better error message
	if (towref<0)
	{
		QString text=tr ("Either the selected flight is neither a towflight nor a towed flight, or it has not departed yet.");
		showWarning (tr ("No towflight"), text, this);
		return;
	}

	// Generate the index in the ObjectListModel
	QModelIndex towrefIndex=index.sibling (towref, index.column ());

	// Jump to the flight
	setCurrentIndex (_proxyModel->mapFromSource (towrefIndex));
}

void FlightTableView::languageChanged ()
{
	if (!_flightList)
		return;

	// See the FlightModel class documentation
	_flightModel->updateTranslations ();
	_flightListModel->reset ();
}

QRectF FlightTableView::rectForFlight (const FlightReference &flight, int column) const
{
	if (!_flightList)
		return QRectF ();

	// Find the index of the flight in the flight proxy list.
	// FIXME use FlightReference
	int flightIndex=_proxyList->modelIndexFor (flight.id (), flight.towflight ());

	// Determine the model index in the flight list model
	QModelIndex modelIndex=_flightListModel->index (flightIndex, column);
	if (!modelIndex.isValid ())
		return QRectF ();

	// Map the model index to the proxy model
	modelIndex=_proxyModel->mapFromSource (modelIndex);
	if (!modelIndex.isValid ())
		return QRectF ();

	// Get the rectangle from the table view
	return visualRect (modelIndex);
}

void FlightTableView::minuteChanged ()
{
	if (!_flightList)
		return;

	QModelIndex oldIndex=currentIndex ();
	QPersistentModelIndex focusWidgetIndex=findButton (
		dynamic_cast<TableButton *> (QApplication::focusWidget ()));

	int durationColumn=_flightModel->durationColumn ();
	_flightListModel->columnChanged (durationColumn);

	// FIXME why do we do this?
	setCurrentIndex (oldIndex);
	focusWidgetAt (focusWidgetIndex);
}

// FIXME add slot

void FlightTableView::base_buttonClicked (QPersistentModelIndex proxyIndex)
{
	if (!_flightList)
		return;

	if (!proxyIndex.isValid ())
	{
		log_error (notr ("A button with invalid persistent index was clicked."));
		return;
	}

	// FIXME use getFlightFor...

	QModelIndex flightListIndex = _proxyModel->mapToSource (proxyIndex);
	const Flight &flight = _flightListModel->at (flightListIndex);

	if (flightListIndex.column () == _flightModel->departButtonColumn ())
		emit departButtonClicked (FlightReference (flight));
	else if (flightListIndex.column () == _flightModel->landButtonColumn ())
		emit landButtonClicked (FlightReference (flight));
	else
		log_error (notr ("Unhandled button column"));
}

void FlightTableView::showNotification (const FlightReference &flight, const QString &message, int milliseconds)
{
	QRectF rect=rectForFlight (flight, 1);

	if (!rect.isValid ())
	{
		return;
	}

	// FIXME behaves badly if the flight is scrolled out of the viewport
	NotificationWidget *nw=new NotificationWidget (viewport ());
	//nw->setDrawWidgetBackground (true);
	nw->setText (message);
	nw->moveArrowTip (rect.center ());
	nw->show ();
	nw->selfDestructIn (milliseconds);
}
