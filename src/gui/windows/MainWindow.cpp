/*
 * Improvements:
 *   - the log should also display queries from other database interfaces that
 *     may be created during connect, such as the root interface
 *   - when double-clicking in the empty area of the flight table, create a new
 *     flight
 *   - when double-clicking the displayed date label, change the displayed date
 */
//	assert (isGuiThread ());

#include "MainWindow.h"

#include <iostream>

#include <QAction>
#include <QSettings>
#include <QEvent>
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
#include "src/i18n/notr.h"
#include "src/gui/widgets/WeatherWidget.h"
#include "src/gui/windows/AboutDialog.h"
#include "src/gui/windows/input/DateInputDialog.h"
#include "src/gui/windows/input/DateTimeInputDialog.h"
#include "src/gui/windows/FlightWindow.h"
#include "src/gui/windows/LaunchMethodSelectionWindow.h"
#include "src/gui/windows/objectList/ObjectListWindow.h"
#include "src/gui/windows/objectList/FlightListWindow.h"
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
#include "src/statistics/FlarmLog.h"
#include "src/gui/dialogs.h"
#include "src/logging/messages.h"
#include "src/util/qString.h"
#include "src/util/qList.h"
#include "src/util/qDate.h"
#include "src/concurrent/monitor/OperationCanceledException.h"
#include "src/db/cache/Cache.h"
#include "src/text.h"
#include "src/i18n/TranslationManager.h"
#include "src/flarm/FlarmHandler.h"

template <class T> class MutableObjectList;

// ******************
// ** Construction **
// ******************

MainWindow::MainWindow (QWidget *parent):
	SkMainWindow<Ui::MainWindowClass> (parent),
	oldLogVisible (false),
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

	// Menu bar
	logAction = ui.logDockWidget->toggleViewAction ();
	ui.menuDatabase->addSeparator ();
	ui.menuDatabase->addAction (logAction);


	connect (&Settings::instance (), SIGNAL (changed ()), this, SLOT (settingsChanged ()));
	readSettings ();
	// This also calls setupText
	settingsChanged ();

	setupLabels ();

	// Info frame
	bool acpiValid = AcpiWidget::valid ();
	ui.powerStateLabel->setVisible (acpiValid);
	ui.powerStateCaptionLabel->setVisible (acpiValid);

	// Change the language every second so we can verify even for modal windows
	// that they are correctly retranslated.
	QTimer *timeTimer = new QTimer (this);
	connect (timeTimer, SIGNAL (timeout ()), this, SLOT (timeTimer_timeout ()));
	timeTimer->start (1000);

	timeTimer_timeout ();

	translationTimer=new QTimer (this);
	connect (translationTimer, SIGNAL (timeout ()), this, SLOT (on_changeLanguageAction_triggered ()));


	setupLayout ();

	// Do this before calling connect
	QObject::connect (&dbManager.getCache (), SIGNAL (changed (DbEvent)), this, SLOT (cacheChanged (DbEvent)));

	QObject::connect (&dbManager, SIGNAL (migrationStarted ()), this, SLOT (migrationStarted ()));
	QObject::connect (&dbManager, SIGNAL (migrationEnded   ()), this, SLOT (migrationEnded   ()));

	// TODO to showEvent?
	QTimer::singleShot (0, this, SLOT (on_actionConnect_triggered ()));

	setDisplayDateCurrent (true);

	ui.logDockWidget->setVisible (false);

	ui.actionShutdown->setVisible (Settings::instance ().enableShutdown);

#if defined (Q_OS_WIN32) || defined (Q_OS_WIN64)
	ui.actionSetTime->setVisible (false);
	ui.actionSetGPSTime->setVisible (false);
	bool virtualKeyboardEnabled=false;
#else
	bool virtualKeyboardEnabled = (
		system (notr ("which kvkbd >/dev/null")) == 0 &&
		system (notr ("which dbus-send >/dev/null")) == 0);
		//system ("which dcop >/dev/null") == 0);
#endif

	ui.actionShowVirtualKeyboard->setVisible (virtualKeyboardEnabled);
//	ui.actionShowVirtualKeyboard->setIcon (QIcon ((const QPixmap&)QPixmap (kvkbd)));
	ui.actionShowVirtualKeyboard->setIcon (QIcon (notr (":/graphics/kvkbd.png")));

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
	
	// Flarm
	FlarmHandler *flarmHandler = FlarmHandler::getInstance ();
	flarmHandler->setDatabase (&dbManager);
	connect (flarmHandler, SIGNAL (actionDetected(const QString&,FlarmHandler::FlightAction)), this, SLOT (onFlarmAction(const QString&, FlarmHandler::FlightAction)));
	connect (flarmHandler, SIGNAL (connectionStateChanged(FlarmHandler::ConnectionState)), this, SLOT (onFlarmConnectionStateChanged(FlarmHandler::ConnectionState)));
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
				if (label->objectName ().contains (notr ("Caption"), Qt::CaseSensitive))
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
	SkLabel *valueLabel = new SkLabel (notr ("..."), ui.pluginPane);

	captionLabel->setText (plugin->getCaption ());

	valueLabel->setWordWrap (true);
	QString toolTip=tr ("%1\nConfiguration: %2").arg (plugin->getDescription (), plugin->configText ());
	valueLabel->setToolTip (toolTip);
	captionLabel->setToolTip (toolTip);

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
		weatherWidget->setText (tr ("Weather"));

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

void MainWindow::restartPlugins ()
{
	if (weatherPlugin) weatherPlugin->restart ();
	if (weatherDialog) weatherDialog->restartPlugin ();

	foreach (InfoPlugin *plugin, infoPlugins)
	{
		plugin->terminate ();
		plugin->start ();
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
	if (!confirmAndExit (0, tr ("Really exit?"), tr ("Really exit the program?")))
		event->ignore ();
}

void MainWindow::on_actionQuit_triggered ()
{
	confirmAndExit (0, tr ("Really exit?"), tr ("Really exit the program?"));
}

void MainWindow::on_actionShutdown_triggered ()
{
	confirmAndExit (69, tr ("Really shut down?"), tr ("Really shut down the computer?"));
}

// **************
// ** Settings **
// **************

void MainWindow::writeSettings ()
{
	QSettings settings;

	settings.beginGroup (notr ("gui"));

	if (fontSet)
	{
		settings.beginGroup (notr ("fonts"));
		QFont font = QApplication::font ();
		settings.setValue (notr ("font"), font.toString ());
		settings.endGroup ();
	}

	settings.beginGroup (notr ("flightTable"));
	ui.flightTable->writeColumnWidths (settings, *flightModel);
	settings.endGroup ();

	settings.endGroup ();

	settings.sync ();
}

void MainWindow::readColumnWidths ()
{
	QSettings settings;

	settings.beginGroup (notr ("gui"));
    settings.beginGroup (notr ("flightTable"));
    ui.flightTable->readColumnWidths (settings, *flightModel);
    settings.endGroup ();
    settings.endGroup ();
}

void MainWindow::readSettings ()
{
	QSettings settings;

	settings.beginGroup (notr ("gui"));
	settings.beginGroup (notr ("fonts"));

	if (settings.contains (notr ("font")))
	{
		QString fontDescription = settings.value (notr ("font")).toString ();
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

/** Setup translated texts */
void MainWindow::setupText ()
{
	Settings &s=Settings::instance ();

	logAction->setText (tr ("Show &log"));

	if (isBlank (s.location))
		setWindowTitle (tr ("Startkladde"));
	else
		setWindowTitle (tr ("Flight log %1 - Startkladde").arg (s.location));

}

void MainWindow::settingsChanged ()
{
	Settings &s=Settings::instance ();

	setupText ();

	ui.menuDebug->menuAction ()->setVisible (s.enableDebug);
	// Even though the menu is invisible, we still need to disable the menu
	// entries to disable their shortcuts.
	ui.changeLanguageAction          ->setEnabled (s.enableDebug);
	ui.timerBasedLanguageChangeAction->setEnabled (s.enableDebug);

	ui.actionNetworkDiagnostics     ->setVisible (!isBlank (s.diagCommand));

	// Plugins
	setupPlugins ();
}

// *************
// ** Flights **
// *************

void MainWindow::updateDisplayDateLabel (const QDate &today)
{
	if (!databaseActionsEnabled)
	{
		ui.displayDateLabel->resetDefaultForegroundColor ();
		ui.displayDateLabel->setText (notr ("-"));
	}
	else if (displayDate==today)
	{
		ui.displayDateLabel->resetDefaultForegroundColor ();
		ui.displayDateLabel->setText (tr ("Today (%1)").arg (today.toString (defaultNumericDateFormat ())));
	}
	else
	{
		ui.displayDateLabel->setPaletteForegroundColor (Qt::red);
		ui.displayDateLabel->setText (dbManager.getCache ().getOtherDate ().toString (tr ("dddd, M/d/yyyy")));
	}
}

/**
 * Refreshes both the flight table and the corresponding info labels, from the
 * cache, including the displayed date label (text and color). Does not access
 * the database.
 */
void MainWindow::refreshFlights ()
{
	// Fetch the current date to avoid it changing during the operation
	// TODO time zone safety: should be local today
	QDate today=QDate::currentDate ();

	QList<Flight> flights;
	if (databaseActionsEnabled)
	{
		if (displayDate==today)
		{
			// The displayed date is today's date - display today's flights and
			// prepared flights
			flights  = dbManager.getCache ().getFlightsToday ().getList ();
			flights += dbManager.getCache ().getPreparedFlights ().getList ();

			proxyModel->setShowPreparedFlights (true);
		}
		else
		{
			// The displayed date is not today's date - display the flights from the
			// cache's "other" date
			flights=dbManager.getCache ().getFlightsOther ().getList ();

			proxyModel->setShowPreparedFlights (false);
		}
	}

	updateDisplayDateLabel (today);

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
		ui.activeFlightsLabel->setText (notr ("-"));

	if (databaseActionsEnabled)
		ui.totalFlightsLabel->setNumber (Flight::countHappened (flights));
	else
		ui.totalFlightsLabel->setText (notr ("-"));
}

// *************************
// ** Flight manipulation **
// *************************

/*
 * Notes:
 *   - for create, repeat and edit: the flight editor may be modeless
 *     and control may return immediately (even for modal dialogs).
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
	try
	{
		dbManager.updateObject (flight, this);
	}
	catch (OperationCanceledException &e)
	{
		// TODO the cache may now be inconsistent
	}
}

bool MainWindow::checkPlaneFlying (dbId id, const QString &description)
{
	if (idValid (id) && cache.planeFlying (id))
	{
		Plane plane=cache.getObject<Plane> (id);
		QString text=tr ("According to the database, the %1 %2 is still flying. Depart anyway?")
				.arg (description, plane.registration);
		if (!yesNoQuestion (this, tr ("Plane still flying"), text))
			return false;
	}

	return true;
}

bool MainWindow::checkPersonFlying (dbId id, const QString &description)
{
	if (idValid (id) && cache.personFlying (id))
	{
		Person person=cache.getObject<Person> (id);
		QString text=tr ("According to the database, the %1 %2 is still flying. Start anyway?")
				.arg (description, person.fullName ());
		if (!yesNoQuestion (this, tr ("Person still flying"), text)) return false;
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
			if (!checkPlaneFlying (flight.getPlaneId (), tr ("plane"))) return;
			if (isAirtow)
				if (!checkPlaneFlying (flight.effectiveTowplaneId (cache), tr ("towplane"))) return;

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
			showWarning (tr ("Departing not possible"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::departFlight").arg (ex.id));
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
			showWarning (tr ("Landing not possible"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight not found in MainWindow::landFlight").arg (ex.id));
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
			showWarning (tr ("Landing not possible"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::landTowFlight").arg (ex.id));
	}
}

void MainWindow::on_actionNew_triggered ()
{
	delete createFlightWindow; // noop if NULL
	createFlightWindow=FlightWindow::createFlight (this, dbManager, getNewFlightDate (), preselectedLaunchMethod);
	createFlightWindow->setAttribute (Qt::WA_DeleteOnClose, true);

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
			tr ("Touch-and-go not possible"),
			tr ("The selected flight is a towflight. Towflights cannot perform a touch-and-go."),
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
			showWarning (tr ("Touch-and-go not possible"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::on_actionTouchngo_triggered").arg (ex.id));
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
		log_error (qnotr ("Flight %1 not found in MainWindow::departOrLand").arg (ex.id));
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
			editFlightWindow->setAttribute (Qt::WA_DeleteOnClose, true);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight not found in MainWindow::on_actionEdit_triggered").arg (ex.id));
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
		showWarning (tr ("Replicating not possible"),
			tr ("The selected flight is a towflight. Towflights cannot be replicated."),
			this);
		return;
	}

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (id);
		delete createFlightWindow; // noop if NULL
		createFlightWindow=FlightWindow::repeatFlight (this, dbManager, flight, getNewFlightDate (), preselectedLaunchMethod);
		createFlightWindow->setAttribute (Qt::WA_DeleteOnClose, true);
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::on_actionRepeat_triggered").arg (id));
	}
}

void MainWindow::on_actionDelete_triggered ()
{
	bool isTowflight = false;
	dbId id = currentFlightId (&isTowflight);

	if (idInvalid (id))
	// TODO display message
	return;

	if (!yesNoQuestion (this, tr ("Delete flight?"), tr ("Really delete flight?"))) return;

	if (isTowflight) if (!yesNoQuestion (this, tr ("Delete glider flight?"),
			tr ("The selected flight is a towflight. Really delete the corresponding glider flight?"))) return;

	try
	{
		// Get the current index
		QModelIndex previousIndex=ui.flightTable->currentIndex ();
		dbManager.deleteObject<Flight> (id, this);
		ui.flightTable->setCurrentIndex (previousIndex); // Handles deletion of last item correctly
	}
	catch (OperationCanceledException &)
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
		showWarning (tr ("No flight selected"), tr ("No flight is selected."), this);
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

		if (error)
		{
			if (isTowflight)
				showWarning (tr ("Towflight has errors"), tr ("First error of the towflight: %1").arg (errorText), this);
			else
				showWarning (tr ("Flight has errors"), tr ("First error of the flight: %1").arg (errorText), this);
		}
		else
		{
			if (isTowflight)
				showWarning (tr ("Towflight has no errors"), tr ("The towflight has no errors."), this);
			else
				showWarning (tr ("Flight has no errors"), tr ("The flight has no errors."), this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::on_actionDisplayError_triggered").arg (ex.id));
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
		int result=system (notr ("dbus-send --print-reply --dest='org.kde.kvkbd' /Kvkbd org.freedesktop.DBus.Properties.Set string:org.kde.kvkbd.Kvkbd string:visible variant:boolean:true >/dev/null"));
		if (result != 0)
		{
			// failed to show; try launch
			if (system (notr ("kvkbd >/dev/null"))==0)
			{
				//system ("dcop kvkbd kvkbd show >/dev/null");
				if (system (notr ("dbus-send --print-reply --dest='org.kde.kvkbd' /Kvkbd org.freedesktop.DBus.Properties.Set string:org.kde.kvkbd.Kvkbd string:visible variant:boolean:true  >/dev/null"))!=0)
					showWarning (tr ("DBus call failed"),
						tr ("The call to dbus-send for displaying the virtual keyboard failed."),
						this);
			}
			else
			{
				showWarning (tr ("Unable to display virtual keyboard"),
					tr ("The virtual keyboard could not be displayed. Maybe kvkbd is not installed."),
					this);
			}

		}
	}
	else
	{
		//system ("/usr/bin/dcop kvkbd kvkbd hide >/dev/null");
		if (system (notr ("dbus-send --print-reply --dest='org.kde.kvkbd' /Kvkbd org.freedesktop.DBus.Properties.Set string:org.kde.kvkbd.Kvkbd string:visible variant:boolean:false >/dev/null"))!=0)
			showWarning (tr ("DBus call failed"),
				tr ("The call to dbus-send for hiding the virtual keyboard failed."),
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
		showWarning (tr ("No flight selected"), tr ("No flight is selected."), this);
		return;
	}

	// Get the towref from the FlightProxyList. The rows of the ObjectListModel
	// correspond to those of its source, the FlightProxyList.
	int towref=proxyList->findTowref (currentIndex.row ());

	// TODO better error message
	if (towref<0)
	{
		QString text=tr ("Either the selected flight is neither a towflight nor a towed flight, or it has not departed yet.");
		showWarning (tr ("No towflight"), text, this);
		return;
	}

	// Generate the index in the ObjectListModel
	QModelIndex towrefIndex=currentIndex.sibling (towref, currentIndex.column ());

	// Jump to the flight
	ui.flightTable->setCurrentIndex (proxyModel->mapFromSource (towrefIndex));
}

void MainWindow::on_actionRestartPlugins_triggered ()
{
	restartPlugins ();
}

// **********
// ** Help **
// **********

void MainWindow::on_actionInfo_triggered ()
{
	AboutDialog aboutDialog (this);
	aboutDialog.setModal (true);
	aboutDialog.exec ();
}

void MainWindow::on_actionNetworkDiagnostics_triggered ()
{
	QString command=Settings::instance ().diagCommand;
	if (isBlank (command)) return;

	// TODO: use QProcess and make sure it's in the background
	if (system (command.toUtf8 ().constData ())!=0)
		showWarning (tr ("Error"),
			tr ("An error occured while executing the network diagnostics command."), this);
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
		// ATTENTION: any changes here should be reflected in the menu entries' text.
		case Qt::Key_F2:  if (databaseActionsEnabled) ui.actionNew           ->trigger (); break;
		case Qt::Key_F3:  if (databaseActionsEnabled) ui.actionRepeat        ->trigger (); break;
		case Qt::Key_F4:  if (databaseActionsEnabled) ui.actionEdit          ->trigger (); break;
		case Qt::Key_F5:  if (databaseActionsEnabled) ui.actionDepart        ->trigger (); break;
		case Qt::Key_F6:  if (databaseActionsEnabled) ui.actionLand          ->trigger (); break;
		case Qt::Key_F7:  if (databaseActionsEnabled) ui.actionTouchngo      ->trigger (); break;
		case Qt::Key_F8:  if (databaseActionsEnabled) ui.actionDelete        ->trigger (); break;
		case Qt::Key_F9:  if (databaseActionsEnabled) ui.actionSort          ->trigger (); break;
		case Qt::Key_F10: if (databaseActionsEnabled) ui.actionSetDisplayDate->trigger (); break;
		case Qt::Key_F11: if (databaseActionsEnabled) ui.actionHideFinished  ->trigger (); break;
		case Qt::Key_F12: if (databaseActionsEnabled) ui.actionRefreshAll    ->trigger (); break;

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
		log_error (notr ("A button with invalid persistent index was clicked in MainWindow::flightTable_buttonClicked"));
		return;
	}

	QModelIndex flightListIndex = proxyModel->mapToSource (proxyIndex);
	const Flight &flight = flightListModel->at (flightListIndex);

//	std::cout << qnotr ("Button clicked at proxy index (%1,%2), flight list index is (%3,%4), flight ID is %5")
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
		std::cerr << notr ("Unhandled button column in MainWindow::flightTable_buttonClicked") << std::endl;
}

#include "src/gui/widgets/TableButton.h"

void MainWindow::updateTimeLabels (const QDateTime &now)
{
	ui.utcTimeLabel  ->setText (now.toUTC       ().toString (defaultNumericDateTimeFormat ()));
	ui.localTimeLabel->setText (now.toLocalTime ().toString (defaultNumericDateTimeFormat ()));
}

void MainWindow::timeTimer_timeout ()
{
	QDateTime now=QDateTime::currentDateTime ();

	updateTimeLabels (now);

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
				weatherDialog->setAttribute (Qt::WA_DeleteOnClose, true);
				weatherDialog->setWindowTitle (s.weatherWindowTitle);
				weatherDialog->show ();
			}
		}
	}
}

// ********************
// ** View - Flights **
// ********************

void MainWindow::on_actionSort_triggered ()
{
	sortCustom ();
}

// **********
// ** Date **
// **********

/**
 * Determines the date to use for new flights
 *
 * This is, depending on the current setting, either the current date (today)
 * or the current displayed date.
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
	// cache's "other" date as displayed date. This would avoid date==today
	// comparisons, prevent inconsistencies and make handling date changes
	// easier (prevent race conditions on date change).

	// Fetch the current date to avoid it changing during the operation
	QDate today=QDate::currentDate ();

	// If the displayed date is null, use the current date
	if (newDisplayDate.isNull ()) newDisplayDate = today;

	// If the displayed date is already current, don't do anything (unless force
	// is true)
	if (newDisplayDate==displayDate && !force) return;

	if (newDisplayDate==today)
	{
		// Setting today's displayed date
		// Since today's flights are always cached, we can do this
		// unconditionally.
		displayDate=newDisplayDate;
	}
	else
	{
		// Setting another date

		try
		{
			// If the new displayed date is not in the cache, fetch it.
			// TODO move that to fetchFlights() (with force flag)
			if (newDisplayDate!=dbManager.getCache ().getOtherDate ())
				dbManager.fetchFlights (newDisplayDate, this);

			// Now the displayed date is the one in the cache (which should be
			// newDisplayDate).
			displayDate=dbManager.getCache ().getOtherDate ();
		}
		catch (OperationCanceledException &ex)
		{
			// The fetching was canceled. Don't change the displayed date.
		}
	}

	// Update the display
	refreshFlights ();
}

void MainWindow::on_actionSetDisplayDate_triggered ()
{
	QDate newDisplayDate = displayDate;
	if (DateInputDialog::editDate (&newDisplayDate, tr ("Set displayed date"), tr ("Displayed date:"), this))
		setDisplayDate (newDisplayDate, true);
}

// ****************
// ** Statistics **
// ****************

// Note that these strings must be defined ouside of the functions because the
// window may have to access them (for retranslation) after the function
// returns.
const char *ntr_planeLogBooksTitle=QT_TRANSLATE_NOOP ("StatisticsWindow", "Plane logbooks");
const char *ntr_pilotLogBooksTitle=QT_TRANSLATE_NOOP ("StatisticsWindow", "Pilot logbooks");
const char *ntr_launchMethodOverviewTitle=QT_TRANSLATE_NOOP ("StatisticsWindow", "Launch method overview");
const char *ntr_flarmOverviewTitle=QT_TRANSLATE_NOOP ("StatisticsWindow", "Flarm overview");

void MainWindow::on_actionPlaneLogs_triggered ()
{
	// Get the list of flights and add the towflights
	QList<Flight> flights=flightList->getList ();
	flights+=Flight::makeTowflights (flights, cache);

	PlaneLog *planeLog = PlaneLog::createNew (flights, cache);
	StatisticsWindow::display (planeLog, true, ntr_planeLogBooksTitle, this);
}

void MainWindow::on_actionPilotLogs_triggered ()
{
	// Get the list of flights and add the towflights
	QList<Flight> flights=flightList->getList ();
	flights+=Flight::makeTowflights (flights, cache);

	// Create the pilots' log
	PilotLog *pilotLog = PilotLog::createNew (flights, cache);

	// Display the pilots' log
	StatisticsWindow::display (pilotLog, true, ntr_pilotLogBooksTitle, this);
}

void MainWindow::on_actionLaunchMethodStatistics_triggered ()
{
	LaunchMethodStatistics *stats = LaunchMethodStatistics::createNew (proxyList->getList (), dbManager.getCache ());
	StatisticsWindow::display (stats, true, ntr_launchMethodOverviewTitle, this);
}

void MainWindow::on_actionFlarmOverview_triggered ()
{
	FlarmLog *flarmLog = FlarmLog::createNew ();
	StatisticsWindow::display (flarmLog, true, ntr_flarmOverviewTitle, this);
}

// **************
// ** Database **
// **************

void MainWindow::on_actionConnect_triggered ()
{
	// Does not throw OperationCanceledException, ConnectionFailedException, ConnectCanceledException, SqlException
	dbManager.connect (this);
}

void MainWindow::on_actionDisconnect_triggered ()
{
	// Does not throw OperationCanceledException, ConnectionFailedException, ConnectCanceledException, SqlException
	dbManager.disconnect ();
}

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
		this);
}

void MainWindow::on_actionShowFlights_triggered ()
{
	if (Settings::instance ().protectFlightDatabase)
	{
		QString password=Settings::instance ().databaseInfo.password;
		QString message=tr ("The database password must be entered to display the flight database.");

		if (!verifyPassword (this, password, message))
			return;
	}

	FlightListWindow::show (dbManager, this);
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
		statusBar ()->showMessage (tr ("Database: %2 (%1)").arg (number).arg (message), 2000);
	}
}

void MainWindow::executingQuery (Query query)
{
	logMessage (query.toString ());
}

void MainWindow::cacheChanged (DbEvent event)
{
	assert (isGuiThread ());

	std::cout << notr ("MainWindow: ") << event.toString () << std::endl;

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
	catch (Cache::NotFoundException &)
	{
		// TODO log error
	}

	// Cannot use foreach because flightList can only return a copy of its list
	int numFlights=flightList->size ();
	for (int i=0; i<numFlights; ++i)
		flightList->at (i).databaseChanged (event);
}

void MainWindow::updateDatabaseStateLabel (DbManager::State state)
{
	switch (state)
	{
		case DbManager::stateDisconnected:
			ui.databaseStateLabel->resetDefaultForegroundColor ();
			ui.databaseStateLabel->setText (tr ("Not connected"));
			break;
		case DbManager::stateConnecting:
			ui.databaseStateLabel->resetDefaultForegroundColor ();
			ui.databaseStateLabel->setText (tr ("Connecting..."));
			break;
		case DbManager::stateConnected:
			ui.databaseStateLabel->resetDefaultForegroundColor ();
			ui.databaseStateLabel->setText (tr ("OK"));
			break;
		// no default
	}
}

void MainWindow::databaseStateChanged (DbManager::State state)
{
	updateDatabaseStateLabel (state);

	switch (state)
	{
		case DbManager::stateDisconnected:
			ui.flightTable->setVisible (false);
			ui.notConnectedPane->setVisible (true);
			setDatabaseActionsEnabled (false);

			refreshFlights (); // Also sets the labels

			break;
		case DbManager::stateConnecting:
			ui.flightTable->setVisible (true);
			ui.notConnectedPane->setVisible (false);
			setDatabaseActionsEnabled (false);

			refreshFlights (); // Also sets the labels

			break;
		case DbManager::stateConnected:
			ui.flightTable->setVisible (true);
			ui.notConnectedPane->setVisible (false);
			setDatabaseActionsEnabled (true);

			setDisplayDateCurrent (true); // Will also call refreshFlights

			ui.flightTable->setFocus ();

			break;
		// no default
	}
}

void MainWindow::onFlarmConnectionStateChanged (FlarmHandler::ConnectionState state) {
	switch (state)
	{
		case FlarmHandler::notConnected:
			ui.flarmStateLabel->setText (tr ("Flarm not connected"));
			break;
		case FlarmHandler::connectedNoData:
			ui.flarmStateLabel->setText (tr ("No Flarm data"));
			break;
		case FlarmHandler::connectedData:
			ui.flarmStateLabel->setText (tr ("Flarm data ok"));
			break;
	}
}

// ***************************
// ** Connection monitoring **
// ***************************

void MainWindow::readTimeout ()
{
	ui.databaseStateLabel->setPaletteForegroundColor (Qt::red);
	ui.databaseStateLabel->setText (tr ("No reply"));
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
	ui.actionShowFlights             ->setEnabled (enabled);
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
		QString title=tr ("Database settings changed");

		QString text=tr (
			"The database settings were changed."
			" The changes will be effective on the next reconnect."
			" Do you want to reconnect now?");

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


	if (DateTimeInputDialog::editDateTime (this, &date, &time, tr ("Set system time")))
	{
		QString timeString=qnotr ("%1-%2-%3 %4:%5:%6")
			.arg (date.year ()).arg (date.month ()).arg (date.day ())
			.arg (time.hour ()).arg (time.minute ()).arg (time.second ());

		// sudo -n: non-interactive (don't prompt for password)
		// sudoers entry: deffi ALL=NOPASSWD: /bin/date

		int result=QProcess::execute (notr ("sudo"), QStringList () << notr ("-n") << notr ("date") << notr ("-s") << timeString);

		if (result==0)
		{
			showWarning (tr ("System time changed"),
				tr ("The system time was changed. The setting"
				" may only be stored permanently when the system is shut down."), this);
		}
		else
		{
			showWarning (tr ("Error"),
				tr ("Changing the system time failed."
				" Maybe the user has insufficient permissions."),
				this);
		}
	}
}

void MainWindow::on_actionSetGPSTime_triggered ()
{
        qDebug () << "MainWindow::on_actionSetGPSTime_triggered" << endl;
        FlarmHandler* flarmHandler = FlarmHandler::getInstance();
        if (flarmHandler->getConnectionState() != FlarmHandler::connectedData) {
                QMessageBox::warning (this, tr("No GPS signal"), "Flarm does not send data");
                return;
        }
        QDateTime current (QDateTime::currentDateTimeUtc ());
        QDateTime currentGPSdateTime = flarmHandler->getGPSTime ();
        qDebug () << "slot_setGPSdateTime: " << currentGPSdateTime.toString ("hh:mm:ss dd.MM.yyyy") << endl;
        qDebug () << "currentTime: " << current.toString ("hh:mm:ss dd.MM.yyyy") << endl;   
        int diff = currentGPSdateTime.secsTo(current);
        if (abs (diff) > 0) {
                if (QMessageBox::question(this, tr("Time difference"), 
                        tr("<p>System time: %1</p>"
                        "<p>GPS time: %2</p>"
                        "<p>The system time differs by %3 seconds from the GPS time.</p>"
                        "<p>Correction?</p>")
                        .arg(current.toString ("hh:mm:ss dd.MM.yyyy"))
                        .arg(currentGPSdateTime.toString ("hh:mm:ss dd.MM.yyyy"))
                        .arg(diff), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                {
        		QString timeString=qnotr ("%1-%2-%3 %4:%5:%6")
	        		.arg (currentGPSdateTime.date().year ()).arg (currentGPSdateTime.date().month ()).arg (currentGPSdateTime.date().day ())
		        	.arg (currentGPSdateTime.time().hour ()).arg (currentGPSdateTime.time().minute ()).arg (currentGPSdateTime.time().second ());

                        // sudo -n: non-interactive (don't prompt for password)
                        // sudoers entry: deffi ALL=NOPASSWD: /bin/date

                        int result=QProcess::execute (notr ("sudo"), QStringList () << notr ("-n") << notr ("date") << notr ("-s") << timeString);

                        if (result==0)
                        {
			        showWarning (tr ("System time changed"),
				        tr ("The system time was changed. The setting"
				        " may only be stored permanently when the system is shut down."), this);
                        }
                        else
                        {
			        showWarning (tr ("Error"),
				        tr ("Changing the system time failed."
				        " Maybe the user has insufficient permissions."),
				        this);
                        }
                }
        }
        else
                QMessageBox::information (this, tr("System time"), tr ("The system time is correct"));

}

void MainWindow::on_actionTest_triggered ()
{
	// Perform a sleep task in the background
}

void MainWindow::logMessage (QString message)
{
	QString timeString = QTime::currentTime ().toString ();

	ui.logWidget->append (qnotr ("[%1] %2").arg (timeString).arg (message));

	// Scroll the log widget to the bottom. Note that ensurecursorVisible
	// may scroll to the bottom right, which is undesirable.
	QScrollBar *scrollBar=ui.logWidget->verticalScrollBar ();
	scrollBar->setValue (scrollBar->maximum ());
}

void MainWindow::on_changeLanguageAction_triggered ()
{
	TranslationManager::instance ().toggleLanguage ();
}

void MainWindow::on_timerBasedLanguageChangeAction_triggered ()
{
	if (ui.timerBasedLanguageChangeAction->isChecked ())
		translationTimer->start (1000);
	else
		translationTimer->stop ();

}

void MainWindow::languageChanged ()
{
	SkMainWindow<Ui::MainWindowClass>::languageChanged ();
	setupText ();

	updateDisplayDateLabel ();
	updateTimeLabels ();
	updateDatabaseStateLabel (dbManager.getState ());

	// See the FlightModel class documentation
	flightModel->updateTranslations ();
	flightListModel->reset ();

	restartPlugins ();

	// Do not call this, we use stored column widths which would be overwritten.
	//ui.flightTable->resizeColumnsToContents ();
}

// Flarm
void MainWindow::onFlarmAction (const QString& flarmid, FlarmHandler::FlightAction action) {
	qDebug () << "MainWindow::onFlarmAction: " << flarmid << "; action = " << action << endl;
	
	Plane plane;
	QString reg;
        dbId planeId = cache.getPlaneIdByFlarmId (flarmid);
        if (idValid (planeId)) {
                plane = cache.getObject<Plane> (planeId);
	        reg = plane.registration; 
                qDebug () << "plane found by flarm id: " << reg << "; " << flarmid << endl;
        }
        else
        {
                qDebug () << "plane not found by flarm id: " << flarmid << endl;
		//TODO: FlarmNet database
		// don't give up. we will create the flight anyway with invalid plane id.
		
		// try to get reg from FlarmNet DB
		/*
		FlarmNetRecord* record = FlarmNetDb::getInstance()->getData (flarmid);
		if (record) {
			// got plane reg from flarmnet. Try to get it from own database
			reg = record->registration.trimmed();
			db->get_plane_registration (&plane, reg);
		}
		else
		*/
                reg = tr ("Unknown plane");
        }                                                

	QList<Flight> flights;
	if (action == FlarmHandler::departure) {
                flights = dbManager.getCache ().getPreparedFlights ().getList ();
                qDebug () << "getPreparedFlights: " << flights.count () << endl;
        }
	else {
                flights = dbManager.getCache ().getFlyingFlights ().getList ();
                qDebug () << "getFlyingFlights: " << flights.count () << endl;
        }

	bool flightFound = false;
	dbId flightId = -1;

        foreach (Flight flight, flights)
        {
                flightId = flight.getId();
                qDebug () << "look for plane id: " << plane.registration << endl;
                if (flight.getPlaneId() == plane.getId()) {
                        flightFound = true;
		        break;
                }
        }
	
	if (flightFound) {
		manipulateFlight (flightId, action);
		//Beep();
		QMessageBox* box = new QMessageBox (QMessageBox::Information, tr ("FLARM Information"),
			tr ("%1 was %2 automatically.").arg(reg).arg(FlarmHandler::flightActionToString(action)),
			QMessageBox::NoButton, this);
		box->setAttribute(Qt::WA_DeleteOnClose);
		// 8 seconds; this is information only
		QTimer::singleShot(8000, box, SLOT(accept()));
		box->show();
	}
	else {
		qDebug () << "no flight found: " << reg << endl;
		// we create the flight with minimal data; will show up in red for completion
		Flight flight;
		flight.setType (FlightBase::typeNormal);
		flight.setPlaneId (plane.getId());
		flight.setMode (FlightBase::modeLocal);
		flight.setFlarmId (flarmid);
		if (action == FlarmHandler::departure)
			flight.setDepartureLocation (Settings::instance ().location);
		else
			flight.setLandingLocation (Settings::instance ().location);

		dbManager.createObject (flight, this);
		manipulateFlight (flight.getId(), action);
		
		QMessageBox* box = new QMessageBox (QMessageBox::Warning, tr ("FLARM Warning"),
			tr ("<qt><p>%1 was %2 automatically.</p>"
			    "<big><font color=\"red\"><p>Entry in flight list is incomplete!</p>"
			    "<p>Please add missing data!</p></font></big></qt>").arg(reg).arg(FlarmHandler::flightActionToString(action)),
			QMessageBox::Ok, 0);
		QTimer::singleShot(10000, box, SLOT(accept()));
		box->show ();
		//manipulateFlight (flight_id, fm_edit);
	}
		
}

void MainWindow::manipulateFlight (dbId flight_id, FlarmHandler::FlightAction action) {
        qDebug () << "MainWindow::manipulateFlight" << endl;
        switch (action)
        {
                case FlarmHandler::departure:
                        departFlight (flight_id);
                        break;
                case FlarmHandler::landing:
                        landFlight (flight_id);
                        break;
                case FlarmHandler::goAround:
                {
                        Flight flight = dbManager.getCache ().getObject<Flight> (flight_id);
                        QString reason;
                        if (flight.canTouchngo (&reason))
                        {
                                flight.performTouchngo ();
                                updateFlight (flight);
                        }
                        else {
                                showWarning (tr ("Touch-and-go not possible"), reason, this);
                        }
                        break;
                }
        }
}
