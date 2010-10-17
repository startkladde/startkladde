/*
 * Improvements:
 *   - the log should also display queries from other database interfaces that
 *     may be created during connect, such as the root interface
 *   - when double-clicking in the empty area of the flight table, create a new
 *     flight
 *   - when double-clicking the display date label, change the display date
 */
//	assert (isGuiThread ());

#include "MainWindow.h"

#include <QAction>
#include <QSettings>
#include <QFontDialog>
#include <QTimer>
#include <QGridLayout>
#include <QProgressDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QList>
#include <QModelIndex>
#include <QStatusBar>
#include <QCloseEvent>
#include <QScrollBar>
#include <QWidget> // remove

// TODO many dependencies - split
#include "src/concurrent/threadUtil.h"
#include "src/config/Settings.h"
#include "src/gui/widgets/WeatherWidget.h"
#include "src/gui/windows/DateInputDialog.h"
#include "src/gui/windows/FlightWindow.h"
#include "src/gui/windows/LaunchMethodSelectionWindow.h"
#include "src/gui/windows/objectList/ObjectListWindow.h"
#include "src/gui/windows/SplashScreen.h"
#include "src/gui/windows/StatisticsWindow.h"
#include "src/gui/windows/WeatherDialog.h"
#include "src/gui/windows/SettingsWindow.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"
#include "src/model/Person.h"
#include "src/model/flightList/FlightModel.h"
#include "src/model/flightList/FlightProxyList.h"
#include "src/model/flightList/FlightSortFilterProxyModel.h"
#include "src/model/objectList/AutomaticEntityList.h" // TODO remove some?
#include "src/model/objectList/EntityList.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/plugin/info/InfoPlugin.h"
#include "src/plugin/weather/WeatherPlugin.h"
#include "src/plugin/factory/PluginFactory.h"
#include "src/statistics/LaunchMethodStatistics.h"
#include "src/statistics/PilotLog.h"
#include "src/statistics/PlaneLog.h"
#include "src/gui/dialogs.h"
#include "src/logging/messages.h"
#include "src/util/qString.h"
#include "src/util/qList.h"
#include "src/concurrent/monitor/OperationCanceledException.h"
#include "src/db/cache/Cache.h"
#include "src/text.h"

template <class T> class MutableObjectList;

// ******************
// ** Construction **
// ******************

MainWindow::MainWindow (QWidget *parent) :
	QMainWindow (parent), oldLogVisible (false),
		dbManager (Settings::instance ().databaseInfo),
		cache (dbManager.getCache ()),
		preselectedLaunchMethod (invalidId),
		createFlightWindow (NULL), editFlightWindow (NULL),
		weatherWidget (NULL), weatherPlugin (NULL),
		weatherDialog (NULL), flightList (new EntityList<Flight> (this)),
		contextMenu (new QMenu (this)),
		databaseActionsEnabled (false),
		fontSet (false)
{
	ui.setupUi (this);

	flightModel = new FlightModel (dbManager.getCache ());
	proxyList=new FlightProxyList (dbManager.getCache (), *flightList, this); // TODO never deleted
	flightListModel = new ObjectListModel<Flight> (proxyList, false, flightModel, true, this);

	proxyModel = new FlightSortFilterProxyModel (dbManager.getCache (), this);
	proxyModel->setSourceModel (flightListModel);

	proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter (true);


	connect (&Settings::instance (), SIGNAL (changed ()), this, SLOT (settingsChanged ()));
	readSettings ();
	settingsChanged ();

	setupLabels ();

	// Info frame
	bool acpiValid = AcpiWidget::valid ();
	ui.powerStateLabel->setVisible (acpiValid);
	ui.powerStateCaptionLabel->setVisible (acpiValid);

	QTimer *timeTimer = new QTimer (this);
	connect (timeTimer, SIGNAL (timeout ()), this, SLOT (timeTimer_timeout ()));
	timeTimer->start (1000);

	timeTimer_timeout ();


	setupLayout ();

	// Do this before calling connect
	QObject::connect (&dbManager.getCache (), SIGNAL (changed (DbEvent)), this, SLOT (cacheChanged (DbEvent)));

	QObject::connect (&dbManager, SIGNAL (migrationStarted ()), this, SLOT (migrationStarted ()));
	QObject::connect (&dbManager, SIGNAL (migrationEnded   ()), this, SLOT (migrationEnded   ()));

	// TODO to showEvent?
	QTimer::singleShot (0, this, SLOT (on_actionConnect_triggered ()));

	setDisplayDateCurrent (true);

	ui.logDockWidget->setVisible (false);

	// Menu bar
	QAction *logAction = ui.logDockWidget->toggleViewAction ();
	logAction->setText ("Protoko&ll anzeigen");
	ui.menuDatabase->addSeparator ();
	ui.menuDatabase->addAction (logAction);

	ui.actionShutdown->setVisible (Settings::instance ().enableShutdown);

#ifdef SK_WINDOWS
	ui.actionSetTime->setVisible (false);
#endif

#if defined(Q_OS_WIN32)
	bool virtualKeyboardEnabled=false;
#elif defined(Q_OS_WIN64)
	bool virtualKeyboardEnabled=false;
#else
	bool virtualKeyboardEnabled = (
		system ("which kvkbd >/dev/null") == 0 &&
		system ("which dbus-send >/dev/null") == 0);
		//system ("which dcop >/dev/null") == 0);
#endif

	ui.actionShowVirtualKeyboard->setVisible (virtualKeyboardEnabled);
//	ui.actionShowVirtualKeyboard->setIcon (QIcon ((const QPixmap&)QPixmap (kvkbd)));
	ui.actionShowVirtualKeyboard->setIcon (QIcon (":/graphics/kvkbd.png"));

	// Log
	ui.logWidget->document ()->setMaximumBlockCount (100);

	// Signals
	connect (flightList, SIGNAL (rowsInserted (const QModelIndex &, int, int)), this, SLOT (flightListChanged ()));
	connect (flightList, SIGNAL (rowsRemoved (const QModelIndex &, int, int)), this, SLOT (flightListChanged ()));
	connect (flightList, SIGNAL (dataChanged (const QModelIndex &, const QModelIndex &)), this, SLOT (flightListChanged ()));
	connect (flightList, SIGNAL (modelReset ()), this, SLOT (flightListChanged ()));

	// Flight table
	ui.flightTable->setAutoResizeRows (true);
	ui.flightTable->setColoredSelectionEnabled (true);
	ui.flightTable->setModel (proxyModel);
	ui.flightTable->resizeColumnsToContents (); // Default sizes

	readColumnWidths (); // Stored sizes


	QObject::connect (
		ui.flightTable, SIGNAL (buttonClicked (QPersistentModelIndex)),
		this, SLOT (flightTable_buttonClicked (QPersistentModelIndex))
		);
	QObject::connect (
		ui.flightTable->horizontalHeader (), SIGNAL (sectionClicked (int)),
		this, SLOT (flightTable_horizontalHeader_sectionClicked (int))
		);

	QObject::connect (ui.actionHideFinished, SIGNAL (toggled (bool)), proxyModel, SLOT (setHideFinishedFlights (bool)));
	QObject::connect (ui.actionAlwaysShowExternal, SIGNAL (toggled (bool)), proxyModel, SLOT (setAlwaysShowExternalFlights (bool)));
	QObject::connect (ui.actionAlwaysShowErroneous, SIGNAL (toggled (bool)), proxyModel, SLOT (setAlwaysShowErroneousFlights (bool)));

	// Initialize all properties of the filter proxy model
	proxyModel->setHideFinishedFlights (ui.actionHideFinished->isChecked ());
	proxyModel->setAlwaysShowExternalFlights (ui.actionAlwaysShowExternal->isChecked ());
	proxyModel->setAlwaysShowErroneousFlights (ui.actionAlwaysShowErroneous->isChecked ());

	proxyModel->setCustomSorting (true);

	ui.flightTable->setFocus ();

	// Database
	connect (&dbManager.getInterface (), SIGNAL (databaseError (int, QString)), this, SLOT (databaseError (int, QString)));
	connect (&dbManager.getInterface (), SIGNAL (executingQuery (Query)), this, SLOT (executingQuery (Query)));

	connect (&dbManager, SIGNAL (readTimeout ()), this, SLOT (readTimeout ()));
	connect (&dbManager, SIGNAL (readResumed ()), this, SLOT (readResumed ()));

	connect (&dbManager, SIGNAL (stateChanged (DbManager::State)), this, SLOT (databaseStateChanged (DbManager::State)));
	databaseStateChanged (dbManager.getState ());
}

MainWindow::~MainWindow ()
{
	// Hide the window to avoid trouble[tm].
	// If we don't make the window invisible here, it will be done in the
	// QWidget destructor. Then the flight table will access its model, which
	// will in turn access the cache, which has already been deleted.
	// In one case, this has been known to lead to a "QMutex::lock: mutex lock
	// failure", but worse things can happen.
	setVisible (false);
	// QObjects will be deleted automatically
	// TODO make sure this also applies to flightList

	terminatePlugins ();
}

void MainWindow::setupLabels ()
{
	if (Settings::instance ().coloredLabels)
	{
		QObjectList labels = ui.infoPane->children ();

		foreach (QObject *object, labels)
		{
			SkLabel *label = dynamic_cast<SkLabel *> (object);
			if (label)
			{
				if (label->objectName ().contains ("Caption", Qt::CaseSensitive))
					label->setPaletteBackgroundColor (QColor (0, 255, 127));
				else
					label->setPaletteBackgroundColor (QColor (0, 127, 255));

				label->setAutoFillBackground (true);
			}
		}
	}
}

void MainWindow::setupLayout ()
{
	// QT 4.3.4 uic ignores the stretch factors (4.5.0 uses it), so set it here.

	QVBoxLayout *centralLayout = (QVBoxLayout *)centralWidget () -> layout ();
	centralLayout->setStretchFactor (ui.topPane, 0);
	centralLayout->setStretchFactor (ui.flightTable, 1);

	//	QHBoxLayout *topPaneLayout     = (QHBoxLayout *) ui.infoPane    -> layout ();
	QHBoxLayout *infoFrameLayout = (QHBoxLayout *)ui.infoFrame -> layout ();
	QGridLayout *infoPaneLayout = (QGridLayout *)ui.infoPane -> layout ();
	QGridLayout *pluginPaneLayout = (QGridLayout *)ui.pluginPane -> layout ();

	/*
	 * This setting gives the plugins more space while still leaving some non-
	 * minimum space between the info and the plugins.
	 * The best behavior would be:
	 *   - normally, info and plugins take up the same amount of space
	 *   - if the plugin values get too large, the info area is made smaller
	 *     in favor of the plugin area
	 *   - if the info area cannot shrink any more, but the plugin area is
	 *     still to small the plugin values are wrapped
	 *   - the plugin values are not wrapped as long as space can be freed by
	 *     shrinking the info area
	 *   - never is the window resized to make space for the plugins if
	 *     something else is possible
	 *   - small changes in the info pane width do not lead to the plugin area
	 *     being moved around
	 */

	// The plugin pane gets twice the space of the info pane
	infoFrameLayout->setStretchFactor (ui.infoPane, 1);
	infoFrameLayout->setStretchFactor (ui.pluginPane, 2);

	// For both the info pane and the plugin pane: all available space goes to
	// the value; the caption is kept at minimum size.
	infoPaneLayout->setColumnStretch (0, 0);
	infoPaneLayout->setColumnStretch (1, 1);
	pluginPaneLayout->setColumnStretch (0, 0);
	pluginPaneLayout->setColumnStretch (1, 1);
}

void MainWindow::setupPlugin (InfoPlugin *plugin, QGridLayout *pluginLayout)
{
	connect (this, SIGNAL (minuteChanged ()), plugin, SLOT (minuteChanged ()));

	SkLabel *captionLabel = new SkLabel ("", ui.pluginPane);
	SkLabel *valueLabel = new SkLabel ("...", ui.pluginPane);

	captionLabel->setText (plugin->getCaption ());

	valueLabel->setWordWrap (true);
	valueLabel->setToolTip (QString ("%1\nKonfiguration: %2").arg (plugin->getDescription (), plugin->configText ()));
	captionLabel->setToolTip (valueLabel->toolTip ());

	int row = pluginLayout->rowCount ();
	pluginLayout->addWidget (captionLabel, row, 0, Qt::AlignTop);
	pluginLayout->addWidget (valueLabel, row, 1, Qt::AlignTop);

	if (Settings::instance ().coloredLabels)
	{
		captionLabel->setPaletteBackgroundColor (QColor (255, 63, 127));
		valueLabel ->setPaletteBackgroundColor (QColor (255, 255, 127));

		captionLabel->setAutoFillBackground (true);
		valueLabel ->setAutoFillBackground (true);
	}

	connect (plugin, SIGNAL (textOutput (QString, Qt::TextFormat)), valueLabel, SLOT (setText (QString, Qt::TextFormat)));

	QObject::connect (captionLabel, SIGNAL (doubleClicked (QMouseEvent *)), plugin, SLOT (restart ()));
	QObject::connect (valueLabel, SIGNAL (doubleClicked (QMouseEvent *)), plugin, SLOT (restart ()));

	plugin->start ();
}

void MainWindow::setupPlugins ()
{
	Settings &s=Settings::instance ();

	// First, terminate the plugins to make sure they won't access the labels
	// any more.
	terminatePlugins ();

	// Remove the old labels from the plugin pane
	foreach (QObject *child, ui.pluginPane->children ())
		delete child;

	// Delete the old layout manager and create a new one, or the layout
	// will be wrong: pluginLayout->rowCount will continue to grow, even
	// though pluginLayout->count will return the correct value.
	delete ui.pluginPane->layout ();
	QGridLayout *pluginLayout=new QGridLayout (ui.pluginPane);
	pluginLayout->setMargin (4);
	pluginLayout->setVerticalSpacing (4);

	deleteList (infoPlugins);

	infoPlugins=s.readInfoPlugins ();

	ui.pluginPane->setVisible (!infoPlugins.isEmpty ());

	foreach (InfoPlugin *plugin, infoPlugins)
		if (plugin->isEnabled ())
			setupPlugin (plugin, pluginLayout);

	pluginLayout->setColumnStretch (0, 0);
	pluginLayout->setColumnStretch (1, 1);
	pluginLayout->setRowStretch (pluginLayout->rowCount (), 1);



	delete weatherWidget;
	weatherWidget=NULL;


	weatherPlugin=NULL; // Deleted in terminatePlugins
	if (s.weatherPluginEnabled && !isBlank (s.weatherPluginId))
		weatherPlugin=PluginFactory::getInstance ().createWeatherPlugin (s.weatherPluginId, s.weatherPluginCommand);

	ui.weatherFrame->setVisible (weatherPlugin!=NULL);
	if (weatherPlugin)
	{
		// Create and setup the weather widget. The weather widget is located to
		// the right of the info frame.
		weatherWidget = new WeatherWidget (ui.weatherFrame);
		ui.weatherFrame->layout ()->addWidget (weatherWidget);
		weatherWidget->setFixedSize (s.weatherPluginHeight, s.weatherPluginHeight);
		weatherWidget->setText ("Wetter");

		weatherPlugin->enableRefresh (s.weatherPluginInterval);
		connect (weatherPlugin, SIGNAL (textOutput (const QString &, Qt::TextFormat)), weatherWidget, SLOT (setText (const QString &, Qt::TextFormat)));
		connect (weatherPlugin, SIGNAL (imageOutput (const QImage &)), weatherWidget, SLOT (setImage (const QImage &)));
		connect (weatherPlugin, SIGNAL (movieOutput (SkMovie &)), weatherWidget, SLOT (setMovie (SkMovie &)));
		connect (weatherWidget, SIGNAL (doubleClicked ()), this, SLOT (weatherWidget_doubleClicked ()));
		weatherPlugin->start ();
	}

}

void MainWindow::terminatePlugins ()
{
	foreach (InfoPlugin *plugin, infoPlugins)
	{
		//std::cout << "Terminating plugin " << plugin->get_caption () << std::endl;
		plugin->terminate ();
		QThread::yieldCurrentThread ();
	}

	while (!infoPlugins.empty ())
		delete infoPlugins.takeLast ();

	if (weatherPlugin)
	{
		weatherPlugin->terminate ();
		delete weatherPlugin;
		weatherPlugin=NULL;
	}
}


// *************
// ** Closing **
// *************

bool MainWindow::confirmAndExit (int returnCode, QString title, QString text)
{
	if (yesNoQuestion (this, title, text))
	{
		closeDatabase ();
		writeSettings ();
		qApp->exit (returnCode);
		return true;
	}
	else
	{
		return false;
	}
}

void MainWindow::closeEvent (QCloseEvent *event)
{
	if (!confirmAndExit (0, "Wirklich beenden?", "Programm wirklich beenden?"))
		event->ignore ();
}

void MainWindow::on_actionQuit_triggered ()
{
	confirmAndExit (0, "Wirklich beenden?", "Programm wirklich beenden?");
}

void MainWindow::on_actionShutdown_triggered ()
{
	confirmAndExit (69, "Wirklich herunterfahren?", "Rechner wirklich herunterfahren?");
}

// **************
// ** Settings **
// **************

void MainWindow::writeSettings ()
{
	QSettings settings;

	settings.beginGroup ("gui");

	if (fontSet)
	{
		settings.beginGroup ("fonts");
		QFont font = QApplication::font ();
		settings.setValue ("font", font.toString ());
		settings.endGroup ();
	}

	settings.beginGroup ("flightTable");
	ui.flightTable->writeColumnWidths (settings, *flightModel);
	settings.endGroup ();

	settings.endGroup ();

	settings.sync ();
}

void MainWindow::readColumnWidths ()
{
	QSettings settings;

	settings.beginGroup ("gui");
    settings.beginGroup ("flightTable");
    ui.flightTable->readColumnWidths (settings, *flightModel);
    settings.endGroup ();
    settings.endGroup ();
}

void MainWindow::readSettings ()
{
	QSettings settings;

	settings.beginGroup ("gui");
	settings.beginGroup ("fonts");

	if (settings.contains ("font"))
	{
		QString fontDescription = settings.value ("font").toString ();
		QFont font;
		if (font.fromString (fontDescription))
		{
			QApplication::setFont (font);
			fontSet=true;
		}
	}

	settings.endGroup ();
	settings.endGroup ();

}

void MainWindow::settingsChanged ()
{
	Settings &s=Settings::instance ();

	// Fenstereinstellungen
	if (isBlank (s.location))
		setWindowTitle ("Startkladde");
	else
		setWindowTitle (utf8 ("Hauptflugbuch %1 - Startkladde").arg (s.location));

	ui.menuDebug     ->menuAction ()->setVisible (Settings::instance ().enableDebug);
	ui.actionNetworkDiagnostics     ->setVisible (!isBlank (Settings::instance ().diagCommand));

	// Plugins
	setupPlugins ();
}

// *************
// ** Flights **
// *************

/**
 * Refreshes both the flight table and the corresponding info labels, from the
 * cache, including the display date label (text and color). Does not access
 * the database.
 */
void MainWindow::refreshFlights ()
{
	// Fetch the current date to avoid it changing during the operation
	// TODO time zone safety: should be local today
	QDate today=QDate::currentDate ();

	QList<Flight> flights;
	if (!databaseActionsEnabled)
	{
		ui.displayDateLabel->resetDefaultForegroundColor ();
		ui.displayDateLabel->setText ("-");
	}
	else if (displayDate==today)
	{
		// The display date is today's date - display today's flights and
		// prepared flights
		flights  = dbManager.getCache ().getFlightsToday ().getList ();
		flights += dbManager.getCache ().getPreparedFlights ().getList ();

		ui.displayDateLabel->resetDefaultForegroundColor ();
		ui.displayDateLabel->setText (utf8 ("Heute (%1)").arg (today.toString (Qt::DefaultLocaleShortDate)));

		proxyModel->setShowPreparedFlights (true);
	}
	else
	{
		// The display date is not today's date - display the flights from the
		// cache's "other" date
		flights=dbManager.getCache ().getFlightsOther ().getList ();

		ui.displayDateLabel->setPaletteForegroundColor (Qt::red);
		ui.displayDateLabel->setText (dbManager.getCache ().getOtherDate ().toString (Qt::DefaultLocaleLongDate));

		proxyModel->setShowPreparedFlights (false);
	}

	bool towflightSelected=false;
	dbId selectedId=currentFlightId (&towflightSelected);
	int column=ui.flightTable->currentIndex ().column ();

	flightList->replaceList (flights);
	sortCustom ();

	if (idValid (selectedId))
		selectFlight (selectedId, towflightSelected, column);

	// TODO should be done automatically
	// ui.flightTable->resizeColumnsToContents ();
	// ui.flightTable->resizeRowsToContents ();

	// TODO: set the cursor to last row, same column as before (this is
	// usually called after a date change, so the previous row is
	// meaningless)
	//int oldColumn = ui.flightTable->currentColumn ();
	//int newRow = ui.flightTable->rowCount () - 1;
	//ui.flightTable->setCurrentCell (newRow, oldColumn);
	// TODO make sure it's visible

	// TODO
	//updateInfo ();
}

dbId MainWindow::currentFlightId (bool *isTowflight)
{
	// Get the currently selected index from the table; it refers to the
	// proxy model
	QModelIndex proxyIndex = ui.flightTable->currentIndex ();

	// Map the index from the proxy model to the flight list model
	QModelIndex flightListModelIndex = proxyModel->mapToSource (proxyIndex);

	// If there is not selection, return an invalid ID
	if (!flightListModelIndex.isValid ()) return invalidId;

	// Get the flight from the model
	const Flight &flight = flightListModel->at (flightListModelIndex);

	if (isTowflight) (*isTowflight) = flight.isTowflight ();
	return flight.getId ();
}

bool MainWindow::selectFlight (dbId id, bool selectTowflight, int column)
{
//	int flightListIndex=flightList->findById (id);

	// Find the flight or towflight with that ID in the flight proxy list
	int proxyListIndex=proxyList->modelIndexFor (id, selectTowflight);
	if (proxyListIndex<0) return false;

	// Create the index in the flight list model
	QModelIndex flightListModelIndex=flightListModel->index (proxyListIndex, column);
	if (!flightListModelIndex.isValid ()) return false;

	// Map the index from the flight list model to the proxy model
	QModelIndex proxyIndex=proxyModel->mapFromSource (flightListModelIndex);
	if (!proxyIndex.isValid ()) return false;

	// Select it
	ui.flightTable->setCurrentIndex (proxyIndex);

	return true;

	// flightList
	// proxyList       = FlightProxyList            (flightList)
	// flightListModel = ObjectListModel            (proxyList)
	// proxyModel      = FlightSortFilterProxyModel (flightListModel)
}

void MainWindow::sortCustom ()
{
	// Use custom sorting
	proxyModel->sortCustom ();

	// Show the sort status in the header view
	ui.flightTable->setSortingEnabled (false); // Make sure it is off
	ui.flightTable->horizontalHeader ()->setSortIndicatorShown (false);
}

void MainWindow::sortByColumn (int column)
{
	// Determine the new sorting order: when custom sorting was in effect or the
	// sorting column changed, sort ascending; otherwise, toggle the sorting
	// order
	if (proxyModel->getCustomSorting ())
		sortOrder=Qt::AscendingOrder; // custom sorting was in effect
	else if (column!=sortColumn)
		sortOrder=Qt::AscendingOrder; // different column
	else if (sortOrder==Qt::AscendingOrder)
		sortOrder=Qt::DescendingOrder; // toggle ascending->descending
	else
		sortOrder=Qt::AscendingOrder; // toggle any->ascending

	// Set the new sorting column
	sortColumn=column;

	// Sort the proxy model
	proxyModel->setCustomSorting (false);
	proxyModel->sort (sortColumn, sortOrder);

	// Show the sort status in the header view
	QHeaderView *header=ui.flightTable->horizontalHeader ();
	header->setSortIndicatorShown (true);
	header->setSortIndicator (sortColumn, sortOrder);
}

/**
 * Called when the flight list changes (flights are inserted or removed or data
 * changes). Updates the info labels.
 */
void MainWindow::flightListChanged ()
{
	QList<Flight> flights=flightList->getList ();

	// Note that there is a race condition if "refresh" is called before
	// midnight and this function is called after midnight.
	// TODO store the todayness somewhere instead.

	if (databaseActionsEnabled && displayDate == QDate::currentDate ())
		ui.activeFlightsLabel->setNumber (Flight::countFlying (flights));
	else
		ui.activeFlightsLabel->setText ("-");

	if (databaseActionsEnabled)
		ui.totalFlightsLabel->setNumber (Flight::countHappened (flights));
	else
		ui.totalFlightsLabel->setText ("-");
}

// *************************
// ** Flight manipulation **
// *************************

/*
 * Notes:
 *   - for create, repeat and edit: the flight editor may be modeless
 *     and control may return immediately (even for modal dialogs). The
 *     The table entry will be updated when the flight editor updates the
 *     database.
 *
 * TODO:
 *   - hier den ganzen Kram wie displayDate==heute und flug schon
 *     gelandet prüfen, damit man die Menüdeaktivierung weglassen kann.
 *     Außerdem kann man dann hier melden, warum das nicht geht.
 *
 */

void MainWindow::updateFlight (const Flight &flight)
{
	// TODO error handling? required? What happens on uncaught exception? Then
	// remove this method if it only calls dbManager.updateObject
	dbManager.updateObject (flight, this);
}

bool MainWindow::checkPlaneFlying (dbId id, const QString &description)
{
	if (idValid (id) && cache.planeFlying (id))
	{
		Plane plane=cache.getObject<Plane> (id);
		QString text=utf8 ("Laut Datenbank fliegt das %1 %2 noch. Trotzdem starten?")
				.arg (description, plane.registration);
		if (!yesNoQuestion (this, "Flugzeug fliegt noch", text))
			return false;
	}

	return true;
}

bool MainWindow::checkPersonFlying (dbId id, const QString &description)
{
	if (idValid (id) && cache.personFlying (id))
	{
		Person person=cache.getObject<Person> (id);
		QString text=utf8 ("Laut Datenbank fliegt der %1 %2 noch. Trotzdem starten?")
				.arg (description, person.fullName ());
		if (!yesNoQuestion (this, "Person fliegt noch", text)) return false;
	}

	return true;
}

void MainWindow::departFlight (dbId id)
{
	// TODO display message
	if (idInvalid (id)) return;

	try
	{
		Flight flight=dbManager.getCache ().getObject<Flight> (id);
		QString reason;

		if (flight.canDepart (&reason))
		{
			bool isAirtow=flight.isAirtow (cache);

			// *** Check for planes flying
			// Plane
			if (!checkPlaneFlying (flight.getPlaneId (), "Flugzeug")) return;
			if (isAirtow)
				if (!checkPlaneFlying (flight.effectiveTowplaneId (cache), "Schleppflugzeug")) return;

			// *** Check for people flying
			// Pilot
			if (!checkPersonFlying (flight.getPilotId (), flight.pilotDescription ())) return;
			// Copilot (if recorded for this flight)
			if (flight.copilotRecorded ())
				if (!checkPersonFlying (flight.getCopilotId (), flight.copilotDescription ())) return;
			// Towpilot (if airtow)
			if (isAirtow && Settings::instance ().recordTowpilot)
				if (!checkPersonFlying (flight.getTowpilotId (), flight.towpilotDescription ())) return;

			flight.departNow ();
			updateFlight (flight);
		}
		else
		{
			showWarning (utf8 ("Start nicht möglich"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (QString ("Flight %1 not found in MainWindow::departFlight").arg (ex.id));
	}
}

void MainWindow::landFlight (dbId id)
{
	// TODO display message
	if (idInvalid (id)) return;

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (id);
		QString reason;

		if (flight.canLand (&reason))
		{
			flight.landNow ();
			updateFlight (flight);
		}
		else
		{
			showWarning (utf8 ("Landung nicht möglich"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (QString ("Flight not found in MainWindow::landFlight").arg (ex.id));
	}
}

void MainWindow::landTowflight (dbId id)
{
	// TODO display message
	if (idInvalid (id)) return;

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (id);
		QString reason;

		if (flight.canTowflightLand (&reason))
		{
			flight.landTowflightNow ();
			updateFlight (flight);
		}
		else
		{
			showWarning (utf8 ("Landung nicht möglich"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (QString ("Flight %1 not found in MainWindow::landTowFlight").arg (ex.id));
	}
}

void MainWindow::on_actionNew_triggered ()
{
	delete createFlightWindow; // noop if NULL
	createFlightWindow=FlightWindow::createFlight (this, dbManager, getNewFlightDate (), preselectedLaunchMethod);
}

void MainWindow::on_actionLaunchMethodPreselection_triggered ()
{
	LaunchMethodSelectionWindow::select (cache, preselectedLaunchMethod, this);
}

void MainWindow::on_actionDepart_triggered ()
{
	// This will check canDepart
	departFlight (currentFlightId ());
}

void MainWindow::on_actionLand_triggered ()
{
	bool isTowflight = false;
	dbId id = currentFlightId (&isTowflight);

	if (isTowflight)
		// This will check canLand
		landTowflight (id);
	else
		landFlight (id);
}

void MainWindow::on_actionTouchngo_triggered ()
{
	bool isTowflight=false;
	dbId id = currentFlightId (&isTowflight);

	if (isTowflight)
	{
		showWarning (
			utf8 ("Zwischenlandung nicht möglich"),
			utf8 ("Der ausgewählte Flug ist ein Schleppflug. Schleppflüge können keine Zwischenlandung machen."),
			this);
	}

	// TODO display message
	if (idInvalid (id)) return;

	try
	{
		// TODO warning if the plane is specified and a glider and the
		// launch method is specified and not an unended airtow

		Flight flight = dbManager.getCache ().getObject<Flight> (id);
		QString reason;

		if (flight.canTouchngo (&reason))
		{
			flight.performTouchngo ();
			updateFlight (flight);
		}
		else
		{
			showWarning (utf8 ("Zwischenlandung nicht möglich"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (QString ("Flight %1 not found in MainWindow::on_actionTouchngo_triggered").arg (ex.id));
	}
}

void MainWindow::departOrLand ()
{
	bool isTowflight=false;
	dbId id = currentFlightId (&isTowflight);

	if (idInvalid (id)) return;

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (id);

		bool flightChanged=true;

		if (flight.canDepart ())
			flight.departNow ();
		else if (isTowflight && flight.canTowflightLand ())
			flight.landTowflightNow ();
		else if (!isTowflight && flight.canLand ())
			flight.landNow ();
		else
			flightChanged=false;

		if (flightChanged)
			updateFlight (flight);
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (QString ("Flight %1 not found in MainWindow::departOrLand").arg (ex.id));
	}
}

void MainWindow::on_actionEdit_triggered ()
{
	dbId id = currentFlightId ();

	if (idInvalid (id)) return;

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (id);

		if (editFlightWindow && editFlightWindow->getEditedId ()==id)
		{
			// The flight is already being edited

			// How to raise a QDialog?
			// How to move to center? editFlightWindow->move does not seem to
			// have any effect (regardless of what - show or move - is done
			// first). Currently, it's done in FlightWindow#showEvent which
			// causes it to be shown in the top-left position first and then
			// moved.
			editFlightWindow->hide ();
			editFlightWindow->show ();
		}
		else
		{
			// Another flight may be being edited
			delete editFlightWindow; // noop if NULL
			editFlightWindow=FlightWindow::editFlight (this, dbManager, flight);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (QString ("Flight not found in MainWindow::on_actionEdit_triggered").arg (ex.id));
	}
}

void MainWindow::on_actionRepeat_triggered ()
{
	bool isTowflight = false;
	dbId id = currentFlightId (&isTowflight);

	// TODO display message
	if (idInvalid (id))
		return;

	else if (isTowflight)
	{
		showWarning (utf8 ("Wiederholen nicht möglich"),
			utf8 ("Der ausgewählte Flug ist ein Schleppflug. Schleppflüge können nicht wiederholt werden."),
			this);
		return;
	}

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (id);
		delete createFlightWindow; // noop if NULL
		createFlightWindow=FlightWindow::repeatFlight (this, dbManager, flight, getNewFlightDate (), preselectedLaunchMethod);
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (QString ("Flight %1 not found in MainWindow::on_actionRepeat_triggered").arg (id));
	}
}

void MainWindow::on_actionDelete_triggered ()
{
	bool isTowflight = false;
	dbId id = currentFlightId (&isTowflight);

	if (idInvalid (id))
	// TODO display message
	return;

	if (!yesNoQuestion (this, utf8 ("Flug löschen?"), utf8 ("Flug wirklich löschen?"))) return;

	if (isTowflight) if (!yesNoQuestion (this, utf8 ("Geschleppten Flug löschen?"), utf8 (
			"Der gewählte Flug ist ein Schleppflug. Soll der dazu gehörige geschleppte Flug wirklich gelöscht werden?"))) return;

	try
	{
		// Get the current index
		QModelIndex previousIndex=ui.flightTable->currentIndex ();
		dbManager.deleteObject<Flight> (id, this);
		ui.flightTable->setCurrentIndex (previousIndex); // Handles deletion of last item correctly
	}
	catch (OperationCanceledException)
	{
		// TODO the cache may now be inconsistent
	}
}

void MainWindow::on_actionDisplayError_triggered ()
{
	// Note: only the first error is displayed

	// TODO: this method is quite complex and duplicates code found
	// elsewhere - the towplane generation should be simplified

	bool isTowflight;
	dbId id = currentFlightId (&isTowflight);

	if (idInvalid (id))
	{
		showWarning (utf8 ("Kein Flug ausgewählt"), utf8 ("Es ist kein Flug ausgewählt"), this);
		return;
	}

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (id);

		Plane *plane=dbManager.getCache ().getNewObject<Plane> (flight.getPlaneId ());
		LaunchMethod *launchMethod=dbManager.getCache ().getNewObject<LaunchMethod> (flight.getLaunchMethodId ());
		Plane *towplane=NULL;

		dbId towplaneId=invalidId;
		if (launchMethod && launchMethod->isAirtow ())
		{
			if (launchMethod->towplaneKnown ())
				towplaneId=dbManager.getCache ().getPlaneIdByRegistration (launchMethod->towplaneRegistration);
			else
				towplaneId=flight.getTowplaneId ();

			if (idValid (towplaneId))
				towplane=dbManager.getCache ().getNewObject<Plane> (towplaneId);
		}

		if (isTowflight)
		{
			dbId towLaunchMethod=dbManager.getCache ().getLaunchMethodByType (LaunchMethod::typeSelf);

			flight=flight.makeTowflight (towplaneId, towLaunchMethod);

			delete launchMethod;
			launchMethod=dbManager.getCache ().getNewObject<LaunchMethod> (towLaunchMethod);

			delete plane;
			plane=towplane;
			towplane=NULL;
		}

		QString errorText;
		bool error=flight.isErroneous (cache, &errorText);

		delete plane;
		delete towplane;
		delete launchMethod;

		QString flightText (isTowflight?"Schleppflug":"Flug");

		if (error)
			showWarning (QString ("%1 fehlerhaft").arg (flightText), QString ("Erster Fehler des %1s: %2").arg (flightText, errorText), this);
		else
			showWarning (QString ("%1 fehlerfrei").arg (flightText), QString ("Der %1 ist fehlerfrei.").arg (flightText), this);
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (QString ("Flight %1 not found in MainWindow::on_actionDisplayError_triggered").arg (ex.id));
	}
}


// **********
// ** Font **
// **********

void MainWindow::on_actionSelectFont_triggered ()
{
	bool ok;
	QFont font = QApplication::font ();
	font = QFontDialog::getFont (&ok, font, this);

	if (ok)
	{
		// The user pressed OK and font is set to the font the user selected
		QApplication::setFont (font);
		fontSet=true;
	}
}

void MainWindow::on_actionIncrementFontSize_triggered ()
{
	QFont font = QApplication::font ();
	int size = font.pointSize ();
	font.setPointSize (size + 1);
	QApplication::setFont (font);
	fontSet=true;
}

void MainWindow::on_actionDecrementFontSize_triggered ()
{
	QFont font = QApplication::font ();
	int size = font.pointSize ();
	if (size>5)
	{
		font.setPointSize (size - 1);
		QApplication::setFont (font);
		fontSet=true;
	}
}

// **********
// ** View **
// **********

void MainWindow::on_actionShowVirtualKeyboard_triggered (bool checked)
{
	if (checked)
	{
		// This call may fail (when the progran is not running), don't display
		// stderr. If it fails, it will be run again.
		// Note that without --print-reply, it doesn't seem to work
		//int result = system ("dcop kvkbd kvkbd show >/dev/null 2>/dev/null");
		int result=system ("dbus-send --print-reply --dest='org.kde.kvkbd' /Kvkbd org.freedesktop.DBus.Properties.Set string:org.kde.kvkbd.Kvkbd string:visible variant:boolean:true >/dev/null");
		if (result != 0)
		{
			// failed to show; try launch
			if (system ("kvkbd >/dev/null")==0)
			{
				//system ("dcop kvkbd kvkbd show >/dev/null");
				if (system ("dbus-send --print-reply --dest='org.kde.kvkbd' /Kvkbd org.freedesktop.DBus.Properties.Set string:org.kde.kvkbd.Kvkbd string:visible variant:boolean:true  >/dev/null")!=0)
					showWarning ("DBus-Aufruf fehlgeschlagen",
						"Der Aufruf von dbus-send zum Anzeigen der Bildschirmtastatur ist fehlgeschlagen.",
						this);
			}
			else
			{
				showWarning ("Bildschirmtastatur konnte nicht angezeigt werden",
					"Die Bildschirmtastatur konnte nicht angezeigt werden. Möglicherweise is kvkbd nicht installiert.",
					this);
			}

		}
	}
	else
	{
		//system ("/usr/bin/dcop kvkbd kvkbd hide >/dev/null");
		if (system ("dbus-send --print-reply --dest='org.kde.kvkbd' /Kvkbd org.freedesktop.DBus.Properties.Set string:org.kde.kvkbd.Kvkbd string:visible variant:boolean:false >/dev/null")!=0)
			showWarning ("DBus-Aufruf fehlgeschlagen",
				"Der Aufruf von dbus-send zum Ausblenden der Bildschirmtastatur ist fehlgeschlagen.",
				this);

	}
}

void MainWindow::on_actionRefreshAll_triggered ()
{
	try
	{
		dbManager.refreshCache (this);
	}
	catch (OperationCanceledException &ex) {}

	refreshFlights ();
}

void MainWindow::on_actionRefreshTable_triggered ()
{
	refreshFlights ();
}

void MainWindow::on_actionJumpToTow_triggered ()
{
	// Get the currently selected index in the ObjectListModel
	QModelIndex currentIndex=proxyModel->mapToSource (ui.flightTable->currentIndex ());

	if (!currentIndex.isValid ())
	{
		showWarning (utf8 ("Kein Flug ausgewählt"), utf8 ("Es ist kein Flug ausgewählt."), this);
		return;
	}

	// Get the towref from the FlightProxyList. The rows of the ObjectListModel
	// correspond to those of its source, the FlightProxyList.
	int towref=proxyList->findTowref (currentIndex.row ());

	if (towref<0)
	{
		QString text=utf8 ("Entweder der gewählte Flug ist weder ein geschleppter Flug noch ein Schleppflug, oder er ist noch nicht gestartet.");
		showWarning (utf8 ("Kein Schleppflug"), text, this);
		return;
	}

	// Generate the index in the ObjectListModel
	QModelIndex towrefIndex=currentIndex.sibling (towref, currentIndex.column ());

	// Jump to the flight
	ui.flightTable->setCurrentIndex (proxyModel->mapFromSource (towrefIndex));
}

void MainWindow::on_actionRestartPlugins_triggered ()
{
	if (weatherPlugin) weatherPlugin->restart ();
	if (weatherDialog) weatherDialog->restartPlugin ();

	foreach (InfoPlugin *plugin, infoPlugins)
	{
		plugin->terminate ();
		plugin->start ();
	}
}

// **********
// ** Help **
// **********

void MainWindow::on_actionInfo_triggered ()
{

	::SplashScreen *splashScreen = new ::SplashScreen (this);
	splashScreen->setAttribute (Qt::WA_DeleteOnClose, true);
	splashScreen->show_version ();

}

void MainWindow::on_actionNetworkDiagnostics_triggered ()
{
	QString command=Settings::instance ().diagCommand;
	if (isBlank (command)) return;

	// TODO: use QProcess and make sure it's in the background
	if (system (command.toUtf8 ().constData ())!=0)
		showWarning (utf8 ("Fehler"),
			utf8 ("Beim Ausführen der Netzwerkdiagnose ist ein Fehler aufgetreten."), this);
}

// ************
// ** Events **
// ************

void MainWindow::keyPressEvent (QKeyEvent *e)
{
//	std::cout << "key " << e->key () << "/" << e->modifiers () << " pressed in MainWindow" << std::endl;

	switch (e->key ())
	{
		// The function keys trigger actions
		case Qt::Key_F2:  if (databaseActionsEnabled) ui.actionNew          ->trigger (); break;
		case Qt::Key_F3:  if (databaseActionsEnabled) ui.actionRepeat       ->trigger (); break;
		case Qt::Key_F4:  if (databaseActionsEnabled) ui.actionEdit         ->trigger (); break;
		case Qt::Key_F5:  if (databaseActionsEnabled) ui.actionDepart       ->trigger (); break;
		case Qt::Key_F6:  if (databaseActionsEnabled) ui.actionLand         ->trigger (); break;
		case Qt::Key_F7:  if (databaseActionsEnabled) ui.actionTouchngo     ->trigger (); break;
		case Qt::Key_F8:  if (databaseActionsEnabled) ui.actionDelete       ->trigger (); break;
		case Qt::Key_F9:  if (databaseActionsEnabled) ui.actionSort         ->trigger (); break;
		case Qt::Key_F10:                             ui.actionQuit         ->trigger (); break;
		case Qt::Key_F11: if (databaseActionsEnabled) ui.actionHideFinished ->trigger (); break;
		case Qt::Key_F12: if (databaseActionsEnabled) ui.actionRefreshAll   ->trigger (); break;

		// Flight manipulation
		// Note that we used to check for ui.flightTable->hasFocus() here, but this prevents
		// the actions from working when a table button is focused.
		case Qt::Key_Insert: if (databaseActionsEnabled) ui.actionNew    ->trigger (); break;
		case Qt::Key_Delete: if (databaseActionsEnabled) ui.actionDelete ->trigger (); break;

		case Qt::Key_Return:
		case Qt::Key_Enter:
			if (databaseActionsEnabled)
			{
				if (e->modifiers () & Qt::ControlModifier)
					ui.actionRepeat ->trigger ();
				else
					ui.actionEdit   ->trigger ();
			}
			break;
		case Qt::Key_Space:
			if (databaseActionsEnabled)
				departOrLand ();

			break;

		default: e->ignore (); break;
	}

	QMainWindow::keyPressEvent (e);
}

/**
 * Not using the activated signal because it may be emitted on single click,
 * depending on the desktop settings.
 */
void MainWindow::on_flightTable_doubleClicked (const QModelIndex &index)
{
	if (index.isValid ())
		ui.actionEdit->trigger ();
	else
		ui.actionNew->trigger ();
}

void MainWindow::on_flightTable_customContextMenuRequested (const QPoint &pos)
{
	contextMenu->clear ();

	if (ui.flightTable->indexAt (pos).isValid ())
	{
		contextMenu->addAction (ui.actionNew);
		contextMenu->addSeparator ();
		contextMenu->addAction (ui.actionDepart);
		contextMenu->addAction (ui.actionLand);
		contextMenu->addAction (ui.actionTouchngo);
		contextMenu->addSeparator ();
		contextMenu->addAction (ui.actionEdit);
		contextMenu->addAction (ui.actionRepeat);
		contextMenu->addAction (ui.actionDelete);
		contextMenu->addSeparator ();
		contextMenu->addAction (ui.actionJumpToTow);
	}
	else
	{
		contextMenu->addAction (ui.actionNew);
	}

	contextMenu->popup (ui.flightTable->mapToGlobal (pos), 0);
}

void MainWindow::flightTable_buttonClicked (QPersistentModelIndex proxyIndex)
{
	if (!proxyIndex.isValid ())
	{
		log_error ("A button with invalid persistent index was clicked in MainWindow::flightTable_buttonClicked");
		return;
	}

	QModelIndex flightListIndex = proxyModel->mapToSource (proxyIndex);
	const Flight &flight = flightListModel->at (flightListIndex);

//	std::cout << QString ("Button clicked at proxy index (%1,%2), flight list index is (%3,%4), flight ID is %5")
//		.arg (proxyIndex.row()).arg (proxyIndex.column())
//		.arg (flightListIndex.row()).arg (flightListIndex.column())
//		.arg (flight.id)
//		<< std::endl;

	if (flightListIndex.column () == flightModel->departButtonColumn ())
		departFlight (flight.getId ());
	else if (flightListIndex.column () == flightModel->landButtonColumn ())
	{
		if (flight.isTowflight ())
			landTowflight (flight.getId ());
		else
			landFlight (flight.getId ());
	}
	else
		std::cerr << "Unhandled button column in MainWindow::flightTable_buttonClicked" << std::endl;
}

QString formatDateTime (const QDateTime &dateTime)
{
	// xx.yy.zzzz aa:bb:cc
	return dateTime.date ().toString (Qt::DefaultLocaleShortDate)+" "+
		dateTime.time ().toString (Qt::DefaultLocaleLongDate);
}

#include "src/gui/widgets/TableButton.h"

void MainWindow::timeTimer_timeout ()
{
	QDateTime now=QDateTime::currentDateTime ();
	ui.utcTimeLabel->setText (formatDateTime (now.toUTC ()));
	ui.localTimeLabel->setText (formatDateTime (now.toLocalTime ()));

	static int lastSecond=0;
	int second=QTime::currentTime ().second ();

	// Some things are done on the beginning of a new minute.
	if (second<lastSecond)
	{
		QModelIndex oldIndex=ui.flightTable->currentIndex ();
		QPersistentModelIndex focusWidgetIndex=ui.flightTable->findButton (
			dynamic_cast<TableButton *> (QApplication::focusWidget ()));

		int durationColumn=flightModel->durationColumn ();
		flightListModel->columnChanged (durationColumn);

		ui.flightTable->setCurrentIndex (oldIndex);
		ui.flightTable->focusWidgetAt (focusWidgetIndex);

		emit minuteChanged ();
	}

	lastSecond=second;
}

void MainWindow::weatherWidget_doubleClicked ()
{
	Settings &s=Settings::instance ();

	if (weatherDialog)
	{
		// How to raise a QDialog?
		weatherDialog->hide ();
		weatherDialog->show ();
	}
	else
	{
		if (s.weatherWindowEnabled && !isBlank (s.weatherWindowPluginId))
		{
			// The plugin will be deleted by the weather dialog
			WeatherPlugin *weatherDialogPlugin=PluginFactory::getInstance ().createWeatherPlugin (s.weatherWindowPluginId, s.weatherWindowCommand);

			if (weatherDialogPlugin)
			{
				weatherDialogPlugin->enableRefresh (s.weatherWindowInterval);

				// The weather dialog will be deleted when it's closed, and
				// weatherDialog is a QPointer, so it will be set to NULL.
				weatherDialog = new WeatherDialog (weatherDialogPlugin, this);
				weatherDialog->setWindowTitle (s.weatherWindowTitle);
				weatherDialog->show ();
			}
		}
	}
}

// ********************
// ** View - Flights **
// ********************

void on_actionSort_triggered ()
{

}

// **********
// ** Date **
// **********

/**
 * Determines the date to use for new flights
 *
 * This is, depending on the current setting, either the current date (today)
 * or the current display date.
 *
 * @return the date to use for new flights
 */
QDate MainWindow::getNewFlightDate ()
{
	if (ui.actionUseCurrentDateForNewFlights->isChecked ())
		return QDate::currentDate ();
	else
		return displayDate;
}


/**
 *
 * @param displayDate null means current
 * @param force true means even if already that date
 */
void MainWindow::setDisplayDate (QDate newDisplayDate, bool force)
{
	// TODO this should be correct now, but it still sucks. The better solution
	// would probably be just to have a flag "displayToday" here, and use the
	// cache's "other" date as display date. This would avoid date==today
	// comparisons, prevent inconsistencies and make handling date changes
	// easier (prevent race conditions on date change).

	// Fetch the current date to avoid it changing during the operation
	QDate today=QDate::currentDate ();

	// If the display date is null, use the current date
	if (newDisplayDate.isNull ()) newDisplayDate = today;

	// If the display date is already current, don't do anything (unless force
	// is true)
	if (newDisplayDate==displayDate && !force) return;

	if (newDisplayDate==today)
	{
		// Setting today's display date
		// Since today's flights are always cached, we can do this
		// unconditionally.
		displayDate=newDisplayDate;
	}
	else
	{
		// Setting another date

		try
		{
			// If the new display date is not in the cache, fetch it.
			// TODO move that to fetchFlights() (with force flag)
			if (newDisplayDate!=dbManager.getCache ().getOtherDate ())
				dbManager.fetchFlights (newDisplayDate, this);

			// Now the display date is the one in the cache (which should be
			// newDisplayDate).
			displayDate=dbManager.getCache ().getOtherDate ();
		}
		catch (OperationCanceledException &ex)
		{
			// The fetching was canceled. Don't change the display date.
		}
	}

	// Update the display
	refreshFlights ();
}

void MainWindow::on_actionSetDisplayDate_triggered ()
{
	QDate newDisplayDate = displayDate;
	if (DateInputDialog::editDate (this, &newDisplayDate, NULL, "Anzeigedatum einstellen", "Anzeigedatum:",
		true, true, true)) setDisplayDate (newDisplayDate, true);
}

// ****************
// ** Statistics **
// ****************

void MainWindow::on_actionPlaneLogs_triggered ()
{
	// Get the list of flights and add the towflights
	QList<Flight> flights=flightList->getList ();
	flights+=Flight::makeTowflights (flights, dbManager.getCache ());

	PlaneLog *planeLog = PlaneLog::createNew (flights, dbManager.getCache ());
	StatisticsWindow::display (planeLog, true, utf8 ("Bordbücher"), this);
}

void MainWindow::on_actionPilotLogs_triggered ()
{
	// Get the list of flights and add the towflights
	QList<Flight> flights=flightList->getList ();
	flights+=Flight::makeTowflights (flights, dbManager.getCache ());

	// Create the pilots' log
	PilotLog *pilotLog = PilotLog::createNew (flights, dbManager.getCache ());

	// Display the pilots' log
	StatisticsWindow::display (pilotLog, true, utf8 ("Flugbücher"), this);
}

void MainWindow::on_actionLaunchMethodStatistics_triggered ()
{
	LaunchMethodStatistics *stats = LaunchMethodStatistics::createNew (proxyList->getList (), dbManager.getCache ());
	StatisticsWindow::display (stats, true, "Startartstatistik", this);
}

// **************
// ** Database **
// **************

void MainWindow::on_actionEditPlanes_triggered ()
{
	ObjectListWindow<Plane>::show (dbManager, this);
}

void MainWindow::on_actionEditPeople_triggered ()
{
	ObjectListWindow<Person>::show (dbManager, this);
}

void MainWindow::on_actionEditLaunchMethods_triggered ()
{
	ObjectListWindow<LaunchMethod>::show (dbManager,
		Settings::instance ().protectLaunchMethods,
		Settings::instance ().databaseInfo.password,
		this);
}


// **************
// ** Database **
// **************

void MainWindow::databaseError (int number, QString message)
{
	if (number==0)
		statusBar ()->clearMessage ();
	else
	{
		logMessage (message);
		statusBar ()->showMessage (utf8 ("Datenbank: %2 (%1)").arg (number).arg (message), 2000);
	}
}

void MainWindow::executingQuery (Query query)
{
	logMessage (query.toString ());
}

void MainWindow::cacheChanged (DbEvent event)
{
	assert (isGuiThread ());

	std::cout << "MainWindow: "<< event.toString () << std::endl;

	try
	{
		// TODO when a plane, person or launch method is changed, the flight list
		// has to be updated, too. But that's a feature of the FlightListModel (?).
		if (event.hasTable<Flight> ())
		{
			switch (event.getType ())
			{
				case DbEvent::typeAdd:
				{
					Flight flight=event.getValue<Flight> ();
					if (flight.isPrepared () || flight.effdatum ()==displayDate)
						flightList->append (flight);

					// TODO: set the cursor position to the flight

					// TODO introduce Flight::hasDate (timeZone)
					if (ui.actionResetDisplayDateOnNewFlight->isChecked ())
					{
						if (flight.isPrepared ())
							setDisplayDateCurrent (false);
						else
							setDisplayDate (flight.effdatum (), false);
					}
				} break;
				case DbEvent::typeChange:
				{
					Flight flight=event.getValue<Flight> ();

					if (flight.isPrepared () || flight.effdatum ()==displayDate)
						flightList->replaceOrAdd (flight.getId (), flight);
					else
						flightList->removeById (flight.getId ());
				} break;
				case DbEvent::typeDelete:
					flightList->removeById (event.getId ());
					break;
			}
		}
	}
	catch (Cache::NotFoundException)
	{
		// TODO log error
	}

	// Cannot use foreach because flightList can only return a copy of its list
	int numFlights=flightList->size ();
	for (int i=0; i<numFlights; ++i)
		flightList->at (i).databaseChanged (event);
}

void MainWindow::databaseStateChanged (DbManager::State state)
{
	switch (state)
	{
		case DbManager::stateDisconnected:
			ui.databaseStateLabel->resetDefaultForegroundColor ();
			ui.databaseStateLabel->setText ("Getrennt");

			ui.flightTable->setVisible (false);
			ui.notConnectedPane->setVisible (true);
			setDatabaseActionsEnabled (false);

			refreshFlights (); // Also sets the labels

			break;
		case DbManager::stateConnecting:
			databaseOkText="Verbindung wird aufgebaut...";
			ui.databaseStateLabel->resetDefaultForegroundColor ();
			ui.databaseStateLabel->setText (databaseOkText);

			ui.flightTable->setVisible (true);
			ui.notConnectedPane->setVisible (false);
			setDatabaseActionsEnabled (false);

			refreshFlights (); // Also sets the labels

			break;
		case DbManager::stateConnected:
			databaseOkText="OK";
			ui.databaseStateLabel->resetDefaultForegroundColor ();
			ui.databaseStateLabel->setText (databaseOkText);

			ui.flightTable->setVisible (true);
			ui.notConnectedPane->setVisible (false);
			setDatabaseActionsEnabled (true);

			setDisplayDateCurrent (true); // Will also call refreshFlights

			ui.flightTable->setFocus ();

			break;
		// no default
	}
}


// ***************************
// ** Connection monitoring **
// ***************************

void MainWindow::readTimeout ()
{
	ui.databaseStateLabel->setPaletteForegroundColor (Qt::red);
	ui.databaseStateLabel->setText ("Keine Antwort");
}

void MainWindow::readResumed ()
{
	ui.databaseStateLabel->resetDefaultForegroundColor ();
	ui.databaseStateLabel->setText (databaseOkText);
}




// ************************************
// ** Database connection management **
// ************************************

void MainWindow::setDatabaseActionsEnabled (bool enabled)
{
	databaseActionsEnabled=enabled;

	ui.actionDelete                  ->setEnabled (enabled);
	ui.actionEdit                    ->setEnabled (enabled);
	ui.actionEditPeople              ->setEnabled (enabled);
	ui.actionEditPlanes              ->setEnabled (enabled);
	ui.actionEditLaunchMethods       ->setEnabled (enabled);
	ui.actionJumpToTow               ->setEnabled (enabled);
	ui.actionLand                    ->setEnabled (enabled);
	ui.actionLaunchMethodStatistics  ->setEnabled (enabled);
	ui.actionNew                     ->setEnabled (enabled);
	ui.actionLaunchMethodPreselection->setEnabled (enabled);
	ui.actionPilotLogs               ->setEnabled (enabled);
	ui.actionPingServer              ->setEnabled (enabled);
	ui.actionPlaneLogs               ->setEnabled (enabled);
	ui.actionRefreshAll              ->setEnabled (enabled);
	ui.actionRefreshTable            ->setEnabled (enabled);
	ui.actionRepeat                  ->setEnabled (enabled);
	ui.actionSetDisplayDate          ->setEnabled (enabled);
	ui.actionDepart                  ->setEnabled (enabled);
	ui.actionTouchngo                ->setEnabled (enabled);
	ui.actionDisplayError            ->setEnabled (enabled);

	ui.flightTable->setEnabled (enabled);

	// Connect/disconnect are special
	ui.actionConnect    ->setEnabled (!enabled);
	ui.actionDisconnect ->setEnabled ( enabled);
}

void MainWindow::closeDatabase ()
{
	// No need, will be closed on destruction
	//dbInterface.close ();
}


// **********
// ** Misc **
// **********

void MainWindow::on_actionSettings_triggered ()
{
	SettingsWindow w (this);
	w.setModal (true); // TODO non-model and auto delete
	w.exec ();


	// Check if the database changed
	if (dbManager.getState ()==DbManager::stateConnected && w.databaseSettingsChanged)
	{
		QString title=utf8 ("Datenbankeinstellungen geändert");

		QString text=utf8 (
			"Die Datenbankeinstellungen wurden geändert."
			" Die Änderungen werden beim nächsten Verbindungsaufbau wirksam."
			" Soll die Verbindung jetzt neu aufgebaut werden?");

		if (yesNoQuestion (this, title, text))
		{
			on_actionDisconnect_triggered ();
			on_actionConnect_triggered ();
		}
	}
}

void MainWindow::on_actionSetTime_triggered ()
{
	// Store the current time to avoid a midnight race condition
	QDateTime oldDateTime = QDateTime::currentDateTime ();

	QDate date = oldDateTime.date ();
	QTime time = oldDateTime.time ();


	if (DateInputDialog::editDate (this, &date, &time, "Systemdatum einstellen", "Datum:", false, false, false))
	{
		QString timeString=QString ("%1-%2-%3 %4:%5:%6")
			.arg (date.year ()).arg (date.month ()).arg (date.day ())
			.arg (time.hour ()).arg (time.minute ()).arg (time.second ());

		// sudo -n: non-interactive (don't propmt for password)
		// sudoers entry: deffi ALL=NOPASSWD: /bin/date

		int result=QProcess::execute ("sudo", QStringList () << "-n" << "date" << "-s" << timeString);

		if (result==0)
		{
			showWarning (utf8 ("Systemzeit geändert"),
				utf8 ("Die Systemzeit wurde geändert. Gegebenenfalls"
				" wird die Änderung erst beim nächsten Herunterfahren"
				" dauerhaft gespeichert."), this);
		}
		else
		{
			showWarning (utf8 ("Fehler"),
				utf8 ("Die Änderung der Systemzeit ist fehlgeschlagen."
				" Möglicherweise sind die Berechtigungen nicht ausreichend."), this);
		}
	}
}

void MainWindow::on_actionTest_triggered ()
{
	// Perform a sleep task in the background
}

void MainWindow::logMessage (QString message)
{
	QString timeString = QTime::currentTime ().toString ();

	ui.logWidget->append (QString ("[%1] %2").arg (timeString).arg (message));

	// Scroll the log widget to the bottom. Note that ensurecursorVisible
	// may scroll to the bottom right, which is undesirable.
	QScrollBar *scrollBar=ui.logWidget->verticalScrollBar ();
	scrollBar->setValue (scrollBar->maximum ());
}
