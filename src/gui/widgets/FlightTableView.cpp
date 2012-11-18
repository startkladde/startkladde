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


// flightList      - Contains the actual flights
// proxyList       = FlightProxyList            (flightList)
//                   Adds towflights. Flight references refer to this list.
// flightListModel = ObjectListModel            (proxyList)
//                   Adds columns (from FlightModel)
// proxyModel      = FlightSortFilterProxyModel (flightListModel)
//                   Sorts and hides finished flights according to the settings.

// Possible improvements:
//   * make sure the notification widget is always in the visible range
//     horizontally (requires notification layout update on horizontal
//     scrolling)

// ******************
// ** Construction **
// ******************

/**
 * Creates a new FlightTableView instance
 *
 * You must call init before any other method is called, including any Qt
 * event handlers. This means that after construction, init must be called
 * before the Qt event loop is reached again. The work of init cannot be
 * performed in the constructor because it needs a parameter which cannot be
 * passed to the constructor in Qt Designer based classes.
 */
FlightTableView::FlightTableView (QWidget *parent):
	SkTableView (parent),
	_dbManager (NULL),
	_flightList (NULL), _proxyList (NULL), _flightModel (NULL),
	_flightListModel (NULL), _proxyModel (NULL),
	_sortColumn (-1), _sortOrder (Qt::AscendingOrder)
{
	// Table header clicked -> toggle sorting order
	connect (horizontalHeader (), SIGNAL (sectionClicked (int)),
		this, SLOT (toggleSorting (int)));

	// Table button clicked
	connect (this, SIGNAL (buttonClicked (QPersistentModelIndex)),
		this, SLOT (base_buttonClicked (QPersistentModelIndex)));
}

FlightTableView::~FlightTableView ()
{
}

/**
 * This method must be called after creation, before any other method is called
 * and before the Qt event loop is reached.
 */
void FlightTableView::init (DbManager *dbManager)
{
	// Don't do this if this instance is already initialized
	if (_dbManager)
		return;

	// Store the DbManager
	_dbManager=dbManager;

	// Note: for the meaning of the numerous models, see the comments at the
	// beginning of the file.

	// Create the proxy list - deleted by this
	_proxyList=new FlightProxyList (dbManager->getCache (), this);

	// Create the flight model - deleted by _flightListModel
	_flightModel = new FlightModel (dbManager->getCache ());

	// Create the flight list model - deleted by this
	_flightListModel = new ObjectListModel<Flight> (_proxyList, false, _flightModel, true, this);

	// Create and setup the flight sort filter proxy model (we'll use this as
	// the table's model) - deleted by this
	_proxyModel = new FlightSortFilterProxyModel (dbManager->getCache (), this);
	_proxyModel->setSourceModel (_flightListModel);
	_proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	_proxyModel->setDynamicSortFilter (true);

	// Connect the signals of the flight sort filter proxy model that
	// necessitate a notification layout update
#define layoutNotificationsOn(signal) connect (_proxyModel, SIGNAL (signal), this, SLOT (layoutNotifications ()));
	layoutNotificationsOn (rowsInserted  (const QModelIndex &, int, int));
	layoutNotificationsOn (rowsRemoved   (const QModelIndex &, int, int));
	layoutNotificationsOn (rowsMoved     (const QModelIndex &, int, int, const QModelIndex &, int));
	layoutNotificationsOn (modelReset    ());
	layoutNotificationsOn (layoutChanged ());
#undef layoutNotificationsOn

	// Set the model
	SkTableView::setModel (_proxyModel);

	// Use a custom sort order
	setCustomSorting ();
}


// ****************
// ** Properties **
// ****************

/**
 * Sets the model to display
 *
 * The model may be NULL to display no data. This class does not take ownership
 * of the model.
 */
void FlightTableView::setModel (EntityList<Flight> *flightList)
{
	// This will cause the view to be reset
	_proxyList->setSourceModel (flightList);

	// Store the model
	_flightList=flightList;
}

/**
 * @see FlightSortFilterProxyModel::setHideFinishedFlights
 */
void FlightTableView::setHideFinishedFlights (bool hideFinishedFlights)
{
	_proxyModel->setHideFinishedFlights (hideFinishedFlights);
}

/**
 * @see FlightSortFilterProxyModel::setAlwaysShowExternalFlights
 */
void FlightTableView::setAlwaysShowExternalFlights (bool alwaysShowExternalFlights)
{
	_proxyModel->setAlwaysShowExternalFlights (alwaysShowExternalFlights);
}

/**
 * @see FlightSortFilterProxyModel::setAlwaysShowExternalFlights
 */
void FlightTableView::setAlwaysShowErroneousFlights (bool alwaysShowErroneousFlights)
{
	_proxyModel->setAlwaysShowErroneousFlights (alwaysShowErroneousFlights);
}

/**
 * @see FlightSortFilterProxyModel::setShowPreparedFlights
 */
void FlightTableView::setShowPreparedFlights (bool showPreparedFlights)
{
	_proxyModel->setShowPreparedFlights (showPreparedFlights);
}


// *******************
// ** Configuration **
// *******************

/**
 * The flight model is use as ColumnInfo.
 *
 * @see SkTableView::readColumnWidths
 */
void FlightTableView::readColumnWidths (QSettings &settings)
{
	SkTableView::readColumnWidths (settings, *_flightModel);
}

/**
 * The flight model is use as ColumnInfo.
 *
 * @see SkTableView::writeColumnWidths
 */
void FlightTableView::writeColumnWidths (QSettings &settings)
{
	SkTableView::writeColumnWidths (settings, *_flightModel);
}

/**
 * Retranslates the models
 */
void FlightTableView::languageChanged ()
{
	// See the FlightModel class documentation
	_flightModel->updateTranslations ();
	_flightListModel->reset ();
}


// ***********
// ** Model **
// ***********

/**
 * Performs tasks that have to be performed at the beginning of each minute,
 * such as updating the flight durations (at the moment, this is the only thing
 * this method does).
 *
 * Call this method when the minute place of the system time changed.
 */
void FlightTableView::minuteChanged ()
{
	QModelIndex oldIndex=currentIndex ();
	QPersistentModelIndex focusWidgetIndex=findButton (
		dynamic_cast<TableButton *> (QApplication::focusWidget ()));

	int durationColumn=_flightModel->durationColumn ();
	_flightListModel->columnChanged (durationColumn);

	// TODO why do we do this? Is it still required?
	setCurrentIndex (oldIndex);
	focusWidgetAt (focusWidgetIndex);
}

/**
 * Creates a model index (in the table view) for the specified column of the
 * specified flight or towflight
 *
 * If the flight reference or column is invalid or out of range, or if the
 * specified flight does not exist or is not visible, an invalid model index is
 * returned.
 */
QModelIndex FlightTableView::getModelIndex (const FlightReference &flightReference, int column) const
{
	// The flight reference refers to the proxy list. Map it to a model index
	// for the proxy model (this view's model) via proxyList, flightListModel
	// and proxyModel.
	// Since all mapping functions can be called with an invalid index and
	// return an invalid index in this case, we don't have to check for
	// intermediate invalid indices.

	int         flightIndex = _proxyList      ->getModelIndex (flightReference);
	QModelIndex modelIndex  = _flightListModel->mapFromSource (flightIndex, column);
	QModelIndex proxyIndex  = _proxyModel     ->mapFromSource (modelIndex);
	return proxyIndex;
}

FlightReference FlightTableView::getFlightReference (const QModelIndex &modelIndex) const
{
	// The model index refers to the proxy model (this view's model). Map it to
	// a flight reference via proxyModel, flightListModel and proxyList.
	// Since all mapping functions can be called with an invalid index and
	// return an invalid index in this case, we don't have to check for
	// intermediate invalid indices.

	QModelIndex     flightListModelIndex = _proxyModel     ->mapToSource        (modelIndex);
	int             proxyListIndex       = _flightListModel->mapToSource        (flightListModelIndex);
	FlightReference flightReference      = _proxyList      ->getFlightReference (proxyListIndex);
	return flightReference;
}

// **********
// ** View **
// **********

/**
 * Returns the rectangle (in the table view) that the specified column of the
 * specified flight or towflight occupies
 *
 * If the flight reference or column is invalid or out of range, or if the
 * specified flight does not exist or is not displayed, an invalid model index
 * is returned. Note that the returned rectangle may be outside of the viewport
 * if the flight is in the table, but scrolled outside of the viewport.
 */
QRectF FlightTableView::rectForFlight (const FlightReference &flight, int column) const
{
	QModelIndex modelIndex=getModelIndex (flight, column);
	if (!modelIndex.isValid ())
		return QRectF ();

	// Get the rectangle from the table view
	return visualRect (modelIndex);
}


// ***************
// ** Selection **
// ***************

/**
 * Returns a flight reference to the currently selected flight or towflight
 *
 * If nothing is selected, an invalid flight reference is returned.
 *
 * @see FlightReference
 */
FlightReference FlightTableView::selectedFlightReference ()
{
	return getFlightReference (currentIndex ());
}

/**
 * Selects the specified column of the specified flight or towflight
 *
 * If the flight reference or column is invalid or out of range, or if the
 * specified flight does not exist or is not visible, nothing happens.
 *
 * Returns true if something was selected (regardless of whether the selection
 * actually changed), or false if not.
 */
bool FlightTableView::selectFlight (const FlightReference &flightReference, int column)
{
	if (!flightReference.isValid ())
		return false;

	// Determine the model index
	QModelIndex index=getModelIndex (flightReference, column);

	// If the model index is invalid, return false
	if (!index.isValid ())
		return false;

	// The model index is valid. Select it and return true.
	setCurrentIndex (index);
	return true;
}

/**
 * Selects the towflight for the currently selected flight, if a towed flight is
 * selected, or vice versa
 *
 * If this is not possible for any reason, a message is shown.
 */
void FlightTableView::interactiveJumpToTowflight ()
{
	// FIXME does it get simpler when we use getFlightReference/getModelIndex?
	// Map the current index to a flight proxy list index
	QModelIndex proxyModelIndex=currentIndex ();
	QModelIndex flightListModelIndex=_proxyModel->mapToSource (proxyModelIndex);
	int flightProxyListIndex=_flightListModel->mapToSource (flightListModelIndex);

	if (flightProxyListIndex<0)
	{
		showWarning (tr ("No flight selected"), tr ("No flight is selected."), this);
		return;
	}

	// Get the flight, we'll need it.
	const Flight &flight=_proxyList->at (flightProxyListIndex);

	// Get the towref from the flight proxy list
	flightProxyListIndex=_proxyList->findTowref (flightProxyListIndex);

	// Special case, this sucks: prepared airtow
	// FIXME make sure we can map an invalid index and check it after mapping.
	if (flight.isAirtow (_dbManager->getCache ()) && flight.isPrepared ())
	{
		QString text=tr ("The flight has not departed yet.");
		showWarning (tr ("No towflight"), text, this);
		return;
	}
	else if (flightProxyListIndex<0)
	{
		// Oops, there is no tow reference. Let's get the flight and see why.
		QString text;
		if (!flight.departsHere ())
			text=tr ("The flight does not depart here.");
		else if (!flight.isTowflight () && !flight.isAirtow (_dbManager->getCache ()))
			text=tr ("The flight is neither a towflight nor a towed flight.");
		else if (!flight.getDeparted ())
			text=tr ("The flight has not departed yet.");
		else
			// Unknown reason
			text=tr ("Towflight/towed flight not found");

		showWarning (tr ("No towflight"), text, this);
		return;
	}

	// Map the flight proxy list index back to a proxy model index and select it
	flightListModelIndex=_flightListModel->mapFromSource (flightProxyListIndex, proxyModelIndex.column ());
	proxyModelIndex=_proxyModel->mapFromSource (flightListModelIndex);
	setCurrentIndex (proxyModelIndex);
}


// *****************
// ** Interaction **
// *****************

/**
 * Emits a specific button click signal for the specified flight
 *
 * Depending on the column part of the model index, either departButtonClicked
 * or landButtonClicked is emitted. The flight is determined based on the row
 * part of the model index.
 */
void FlightTableView::base_buttonClicked (QPersistentModelIndex proxyIndex)
{
	// FIXME simplify - don't need to fetch the flight?
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
 * Sets the sort criterion and order and updates the sort indicator in the
 * column header
 *
 * If column>=0, the flights are sorted by the specified column with the
 * specified sort order. If column<0, the flights are sorted by effective time
 * and the sort order is ignored.
 *
 * Note: always use this method rather than sortByColumn.
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

/**
 * A shortcut to setSorting for sorting the flights by effective time
 */
void FlightTableView::setCustomSorting ()
{
	setSorting (-1, Qt::AscendingOrder);
}

/**
 * Toggles sorting between ascending, descending and effective time
 *
 * If sorting is by effective time or by a different column, it is set to the
 * specified column in ascending order. If it is the same column in ascending
 * order, it is set to descending order. If it is the same column in descending
 * order, it is set to effective time.
 */
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


// *******************
// ** Notifications **
// *******************

// Note that it would be convenient to use a QPersistenModelIndex to track the
// index of a flight, so we wouldn't have to look up the index (with all the
// proxy mappings) each time the model changes. However, when a flight is
// hidden, the QPersistenModelIndex will become invalid and stay invalid even
// if the flight is shown again later. Therefore, we have to resort to tracking
// the flight ID (using a FlightReference) instead.

/**
 * Lays out the notification widgets next to their respective flights
 *
 * This method must be called whenever flights are shown, hidden or moved in the
 * flight table. It must also be called whenever a notification is added or
 * removed because notification positions can depend on each other (e. g. to
 * avoid overlap, although that isn't implemented at the moment).
 *
 * The method does not have to be called when the table is scrolled, as all
 * widgets on the viewport are scrolled automatically by QAbstractScrollArea.
 */
void FlightTableView::layoutNotifications ()
{
	// FIXME prevent overlaps (require at least one pixel between adjacent
	// widgets). Actual least squares layout is probably not easy to do, so we
	// might just stick with placing widgets from top to bottom. The widgets
	// will have to support a more general arrow/bubble placement scheme,
	// though. Also, not that we have to consider the bubble height, rather than
	// the widget height, which is important if the arrow points to a position
	// above or below the bubble.
	// A further improvement would be to make sure that all bubbles are actually
	// inside the viewport; if a flight is not visible (because it's scrolled
	// outside of the viewport), the corresponding bubble should show no arrow
	// (or an arrow pointing vaguely up/downward) and a different text ("the
	// flight of D-xxxx was..." instead of "the flight was...").

	QHashIterator<NotificationWidget *, FlightReference> i (notifications);
	while (i.hasNext ())
	{
		i.next ();

		NotificationWidget *widget=i.key ();
		FlightReference flight=i.value ();

		QRectF rect=rectForFlight (flight, _flightModel->pilotColumn ());
		if (rect.isValid ())
		{
			double arrowX=rect.right ()-rect.height ()/2;
			double arrowY=rect.top   ()+rect.height ()/2;
			widget->moveTo (QPointF (arrowX, arrowY));
			//widget->moveTo (QPointF (arrowX, arrowY), QPointF (arrowX+20, 50));
			widget->show ();
		}
		else
		{
			widget->hide ();
		}
	}
}

/**
 * Removes a notification widget after it has been closed
 *
 * This slot must be connected to the closed() slot of a notification widget.
 * This method removes the widget and deletes it. After that, the remaining
 * notifications widgets (if any) are layed out again.
 */
void FlightTableView::notificationWidget_closed ()
{
	// Remove the widget from the list and delete it
	NotificationWidget *widget=dynamic_cast<NotificationWidget *> (sender ());
	FlightReference flight=notifications.take (widget);
	widget->deleteLater ();

	// Don't keep the flight open
	_proxyModel->setForceVisible (FlightReference (flight), false);

	// Layout all widgets
	layoutNotifications ();
}

/**
 * Creates and displays a new notification widget
 *
 * The notification widget will be associated with the specified flight. It will
 * show the specified message and will be closed after the specified time (in
 * milliseconds).
 *
 * This class takes ownership of the notification widget. It will be deleted
 * by the notificationWidget_closed slot.
 */
void FlightTableView::showNotification (const FlightReference &flight, const QString &message, int milliseconds)
{
	// Create and setup the widget. The widget will be deleted by this class
	// when it is closed, which happens after a delay oder when the user clicks
	// the widget.
	NotificationWidget *notificationWidget=new NotificationWidget (viewport ());
	notificationWidget->setAutoFillBackground (true);
	notificationWidget->setText (message);
	notificationWidget->fadeOutAndCloseIn (milliseconds);

	// Notify this when the widget is closed
	connect (notificationWidget, SIGNAL (closed ()), this, SLOT (notificationWidget_closed ()));

	// Add the widget to the list
	notifications.insert (notificationWidget, flight);

	// Layout all widgets
	layoutNotifications ();

	// Keep the flight open. Note that we do that after creating the
	// notification widget in order to cause a re-layout to work around a
	// NotificationWidget bug (see NotificationWidget).
	_proxyModel->setForceVisible (FlightReference (flight), true);
}
