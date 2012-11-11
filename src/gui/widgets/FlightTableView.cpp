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

/**
 * You must call init before any other method is called, including any Qt
 * events. This means that after construction, init must be called before the
 * Qt event loop is reached again.
 */
FlightTableView::FlightTableView (QWidget *parent):
	SkTableView (parent),
	_dbManager (NULL),
	_flightList (NULL), _proxyList (NULL), _flightModel (NULL),
	_flightListModel (NULL), _proxyModel (NULL)
{
	connect (horizontalHeader (), SIGNAL (sectionClicked (int)),
		this, SLOT (toggleSorting (int)));

	connect (
		this, SIGNAL (buttonClicked (QPersistentModelIndex)),
		this, SLOT (base_buttonClicked (QPersistentModelIndex))
		);
}

FlightTableView::~FlightTableView ()
{
	delete _flightModel;
}

void FlightTableView::init (DbManager *dbManager)
{
	if (_dbManager)
		return;

	_dbManager=dbManager;

	_proxyList=new FlightProxyList (dbManager->getCache (), this);

	_flightModel = new FlightModel (dbManager->getCache ());

	_flightListModel = new ObjectListModel<Flight> (_proxyList, false, _flightModel, true, this);

	_proxyModel = new FlightSortFilterProxyModel (dbManager->getCache (), this);
	_proxyModel->setSourceModel (_flightListModel);
	_proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	_proxyModel->setDynamicSortFilter (true);

	SkTableView::setModel (_proxyModel);

	connect (_proxyModel, SIGNAL (rowsInserted  (const QModelIndex &, int, int)), this, SLOT (layoutNotifications ()));
	connect (_proxyModel, SIGNAL (rowsMoved (const QModelIndex &, int, int, const QModelIndex &, int)), this, SLOT (layoutNotifications ()));
	connect (_proxyModel, SIGNAL (rowsRemoved (const QModelIndex &, int, int)), this, SLOT (layoutNotifications ()));
	connect (_proxyModel, SIGNAL (modelReset ()), this, SLOT (layoutNotifications ()));
	connect (_proxyModel, SIGNAL (layoutChanged ()), this, SLOT (layoutNotifications ()));

	setCustomSorting ();
}


// ****************
// ** Properties **
// ****************

void FlightTableView::setModel (EntityList<Flight> *flightList)
{
	// This will cause the view to be reset
	_proxyList->setSourceModel (flightList);
}

void FlightTableView::setHideFinishedFlights (bool hideFinishedFlights)
{
	_proxyModel->setHideFinishedFlights (hideFinishedFlights);
}

void FlightTableView::setAlwaysShowExternalFlights (bool alwaysShowExternalFlights)
{
	_proxyModel->setAlwaysShowExternalFlights (alwaysShowExternalFlights);
}

void FlightTableView::setAlwaysShowErroneousFlights (bool alwaysShowErroneousFlights)
{
	_proxyModel->setAlwaysShowErroneousFlights (alwaysShowErroneousFlights);
}

void FlightTableView::setShowPreparedFlights (bool showPreparedFlights)
{
	_proxyModel->setShowPreparedFlights (showPreparedFlights);
}


// *******************
// ** Configuration **
// *******************

void FlightTableView::readColumnWidths (QSettings &settings)
{
	SkTableView::readColumnWidths (settings, *_flightModel);
}

void FlightTableView::writeColumnWidths (QSettings &settings)
{
	SkTableView::writeColumnWidths (settings, *_flightModel);
}

void FlightTableView::languageChanged ()
{
	// See the FlightModel class documentation
	_flightModel->updateTranslations ();
	_flightListModel->reset ();
}


// ***********
// ** Model **
// ***********

void FlightTableView::minuteChanged ()
{
	QModelIndex oldIndex=currentIndex ();
	QPersistentModelIndex focusWidgetIndex=findButton (
		dynamic_cast<TableButton *> (QApplication::focusWidget ()));

	int durationColumn=_flightModel->durationColumn ();
	_flightListModel->columnChanged (durationColumn);

	// TODO why do we do this?
	setCurrentIndex (oldIndex);
	focusWidgetAt (focusWidgetIndex);
}


// **********
// ** View **
// **********

FlightReference FlightTableView::getCurrentFlightReference ()
{
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
	if (!flightReference.isValid ())
		return false;

	// Find the flight or towflight with that ID in the flight proxy list
	int proxyListIndex=_proxyList->modelIndexFor (flightReference);
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

void FlightTableView::interactiveJumpToTowflight ()
{
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

QModelIndex FlightTableView::modelIndexForFlight (const FlightReference &flight, int column) const
{
	// Find the index of the flight in the flight proxy list.
	int flightIndex=_proxyList->modelIndexFor (flight);

	// Determine the model index in the flight list model
	QModelIndex modelIndex=_flightListModel->index (flightIndex, column);
	if (!modelIndex.isValid ())
		return QModelIndex ();

	// Map the model index to the proxy model
	return _proxyModel->mapFromSource (modelIndex);
}

QRectF FlightTableView::rectForFlight (const FlightReference &flight, int column) const
{
	QModelIndex modelIndex=modelIndexForFlight (flight, column);
	if (!modelIndex.isValid ())
		return QRectF ();

	// Get the rectangle from the table view
	return visualRect (modelIndex);
}

void FlightTableView::base_buttonClicked (QPersistentModelIndex proxyIndex)
{
	if (!proxyIndex.isValid ())
	{
		log_error (notr ("A button with invalid persistent index was clicked."));
		return;
	}

	QModelIndex flightListIndex = _proxyModel->mapToSource (proxyIndex);
	const Flight &flight = _flightListModel->at (flightListIndex);

	if (flightListIndex.column () == _flightModel->departButtonColumn ())
		emit departButtonClicked (FlightReference (flight));
	else if (flightListIndex.column () == _flightModel->landButtonColumn ())
		emit landButtonClicked (FlightReference (flight));
	else
		log_error (notr ("Unhandled button column"));
}


// *************
// ** Sorting **
// *************

/**
 * Don't use sortByColumn
 */
void FlightTableView::setSorting (int column, Qt::SortOrder order)
{
	// Make sure QTableView's sorting mechanism is disabled, we use our own
	// sorting mechanism
	setSortingEnabled (false);

	if (column>=0)
	{
		// Regular sorting
		_proxyModel->setCustomSorting (false);
		_proxyModel->sort (column, order);

		// Show the sort status in the header view
		QHeaderView *header=horizontalHeader ();
		header->setSortIndicatorShown (true);
		header->setSortIndicator (column, order);
	}
	else
	{
		// Custom sorting
		_proxyModel->sortCustom ();

		// Hide the sort status in the header view
		horizontalHeader ()->setSortIndicatorShown (false);
	}

	// Store the current sort order
	_sortColumn=column;
	_sortOrder=order;
}

void FlightTableView::setCustomSorting ()
{
	setSorting (-1, Qt::AscendingOrder);
}

void FlightTableView::toggleSorting (int column)
{
	// Toggle sorting: custom -> ascending -> descending
	if (_sortColumn<0)
		// custom -> ascending
		setSorting (column, Qt::AscendingOrder);
	else if (_sortColumn==column && _sortOrder==Qt::AscendingOrder)
		// same column ascending -> descending
		setSorting (column, Qt::DescendingOrder);
	else if (_sortColumn==column && _sortOrder==Qt::DescendingOrder)
		// same column descending -> custom
		setSorting (-1, Qt::AscendingOrder);
	else
		// different column or unhandled order -> ascending
		setSorting (column, Qt::AscendingOrder);
}


// *******************************************
// ** FlightTableView::Notification methods **
// *******************************************

FlightTableView::Notification::Notification (NotificationWidget *widget,
	const FlightReference &flight):
	widget (QWeakPointer<NotificationWidget> (widget)),
	flight (flight)
{
}


// *******************
// ** Notifications **
// *******************

// Note that it would be nice to use a QPersistenModelIndex to track the index
// of a flight, so we wouldn't have to look up the index (with all the proxy
// mappings) each time the model changes. However, when a flight is hidden, the
// QPersistenModelIndex will become invalid and stay invalid even if the flight
// is shown again later. Therefore, we have to resort to tracking the flight ID
// (using a FlightReference) instead.

/**
 * Lays out the notification widgets next to their respective flights
 *
 * This method must be called whenever flights are shown, hidden or moved in the
 * flight table.
 *
 * This method checks all notifications that are currently in the list. If the
 * widget has been deleted (because it self-destructed, it was closed by the
 * user, or for any other reason), the notification is removed from the list.
 * Otherwise, its position is updated.
 *
 * The widget can be deleted in response to a Qt event (user interaction or
 * timer). This can not happen asynchronously (i. e. from a different thread),
 * so we do not have to cast the QWeakPointer to a QSharedPointer.
 *
 * However, since the order of events is not defined, it is possible that, when
 * a widget is destroyed, this method is called before the weak pointer is
 * invalidated. In this case, the widget (which is still valid) will be layed
 * out again and the notification will only be deleted on the next invocation of
 * this method.
 *
 * Note that ideally, this method would also make sure that notification widgets
 * don't overlap (although this is currently not implemented). Therefore, this
 * method is also called when a new notification is displayed or a notification
 * is removed, even though it currently does not affect the positions of other
 * notifications.
 *
 * When we implement non-overlap, we should also make sure that the widget is
 * actually destroyed (or at least hidden) before this method is invoked.
 */
void FlightTableView::layoutNotifications ()
{
	QMutableListIterator<Notification> i (notifications);
	while (i.hasNext ())
	{
		i.next ();
		QWeakPointer<NotificationWidget> notificationWidget=i.value ().widget;
		FlightReference flight=i.value ().flight;

		// If the notification widget was deleted in the meantime, delete the
		// hash entry. Otherwise, update widget position.
		if (notificationWidget)
		{
			QRectF rect=rectForFlight (flight, 1);
			if (rect.isValid ())
			{
				notificationWidget.data ()->moveArrowTip (rect.center ());
				notificationWidget.data ()->show ();
			}
			else
			{
				notificationWidget.data ()->hide ();
			}
		}
		else
		{
			i.remove ();
		}
	}
}

void FlightTableView::showNotification (const FlightReference &flight, const QString &message, int milliseconds)
{
	NotificationWidget *notificationWidget=new NotificationWidget (viewport ());

	//widget->setDrawWidgetBackground (true);
	notificationWidget->setText (message);
	notificationWidget->selfDestructIn (milliseconds);

	notifications.append (Notification (notificationWidget, flight));
	connect (notificationWidget, SIGNAL (destroyed ()), this, SLOT (layoutNotifications ()));
	layoutNotifications ();
}
