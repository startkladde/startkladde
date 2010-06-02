#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
 * Remake:
 *   - TODO incoming motor flight without pilot is error in table, not in
 *     editor
 *   - TODO towplane type and registration not shown correctly
 *   - TODO the weather widget is still made smaller if the window is resized.
 *          Note that it will not get small than the fixed size set in the UI.
 *   - TODO use QDate or date?
 *   - TODO table sorting
 *   - TODO change the display when the database does not reply to ping
 *   - TODO task error handling
 *   - TODO warn on edit if database not alive (recently responded)
 *   - TODO on startup, display only one progress dialog with multiple items
 *     which are checked after they are completed
 *
 * Tests:
 *   - TODO context menu: correct flight used
 *
 * Further improvements:
 *   - on repeating a towflight, ask if the towed flight should be repeated
 *   - change menu entries for "jump to towflight/towed flight" and "land/
 *     /end airtow" depending on the selected flight.
 *   - update action state if the virtual keyboard is closed/killed externally
 *   - reload weather plugin from context menu
 *   - addObject the weather plugin in designer (need different initialization)
 *   - addObject a menu entry for opening the weather dialog
 *   - focus the weather dialog instead of hiding and showing it
 *   - on date wrap, the display date label's color is not updated
 *   - double click in free table space: create new flight
 *   - make a script for date changing (set date and store to hardware clock)
 *   - when performing a touchngo with a towflight, land and depart it
 *   - allow repeating of towflights
 */

#include <QtGui/QMainWindow>
#include <QPointer>
#include <QPersistentModelIndex>

#include "ui_MainWindow.h"

#include "src/db/DbManager.h"

class QWidget;
template<class T> class QList;
class QModelIndex;
class FlightSortFilterProxyModel;
class ShellPlugin;
class ShellPluginInfo;
class WeatherWidget;
class WeatherDialog;
class FlightModel;
class FlightProxyList;
class FlightWindow;
template<class T> class ObjectListModel;

/*
 * Notes:
 *   - We don't enable/disable the flight manipulation menu entries depending
 *     on whether the manipulation can be performed. It has to be checked when
 *     it's performed anyway, and that way the user can be told why someting
 *     is not possible.
 */
class MainWindow: public QMainWindow
{
	Q_OBJECT

	public:
		MainWindow (QWidget *parent);
		~MainWindow ();

	protected:
		// Startup/Shutdown
		bool confirmAndExit (int returnCode, QString title, QString text);

		// Setup
		void setupPlugins ();
		void setupLabels ();
		void setupLayout ();

		// Actions
		void refreshFlights ();

		// Data
		QDate getNewFlightDate ();
		void setDisplayDate (QDate displayDate, bool force);
		void setDisplayDateCurrent (bool force) { setDisplayDate (QDate::currentDate (), force); }
		void updateFlight (const Flight &flight);

		dbId currentFlightId (bool *isTowflight=NULL);
		void sortCustom ();
		void sortByColumn (int column);

		// Settings
		void writeSettings ();
		void readSettings ();
		void readColumnWidths ();

		// Events
		void closeEvent (QCloseEvent *event);
		void keyPressEvent (QKeyEvent *);

		// Database connection management
		void closeDatabase ();
		void setDatabaseActionsEnabled (bool enabled);

		// Plugins
		void setupPlugin (const ShellPluginInfo &pluginInfo, QGridLayout *pluginLayout);
		void terminatePlugins ();


	protected slots:
		void databaseError (int number, QString message);
		void databaseStateChanged (DbManager::State state);
		void readTimeout ();
		void readResumed ();

		void migrationStarted () { oldLogVisible=ui.logDockWidget->isVisible (); ui.logDockWidget->setVisible (true); }
		void migrationEnded () { ui.logDockWidget->setVisible (oldLogVisible); }

		void settingsChanged ();

	private slots:
		// Menu: Program
		void on_actionSettings_triggered ();
		void on_actionSetTime_triggered ();
		void on_actionQuit_triggered ();
		void on_actionShutdown_triggered ();

		// Menu: Flight
		void on_actionNew_triggered ();
		void on_actionLaunchMethodPreselection_triggered ();
		void on_actionDepart_triggered ();
		void on_actionLand_triggered ();
		void on_actionTouchngo_triggered ();
		void on_actionEdit_triggered ();
		void on_actionRepeat_triggered ();
		void on_actionDelete_triggered ();
		void on_actionDisplayError_triggered ();

		// Menu: View
		void on_actionRefreshTable_triggered ();
		//        void on_actionFont_triggered ();
		void on_actionJumpToTow_triggered ();
		void on_actionRestartPlugins_triggered ();
		void on_actionShowVirtualKeyboard_triggered (bool checked);

		// Menu: View - Font
		void on_actionSelectFont_triggered ();
		void on_actionIncrementFontSize_triggered ();
		void on_actionDecrementFontSize_triggered ();

		// Menu: View - Flights
		// on_actionHideFinished_triggered (bool checked) - connected to proxyModel
		// on_actionAlwaysShowExternal_triggered (bool checked) - connected to proxyModel
		// on_actionAlwaysShowErroneous_triggered (bool checked) - connected to proxyModel
		void on_actionSort_triggered () { sortCustom (); }
		void on_actionResizeColumns_triggered () { ui.flightTable->resizeColumnsToContents (); }


		// Menu: View - Date
		void on_actionSetDisplayDate_triggered ();
		//		void on_actionResetDisplayDateOnNewFlight_triggered ();
		//		void on_actionUseCurrentDateForNewFlights_triggered ();

		// Menu: Statistics
		void on_actionPlaneLogs_triggered ();
		void on_actionPersonLogs_triggered ();
		void on_actionLaunchMethodStatistics_triggered ();

		// Menu: Database
		void on_actionConnect_triggered () { dbManager.connect (this); }
		void on_actionDisconnect_triggered () { dbManager.disconnect (); }
		void on_actionEditPlanes_triggered ();
		void on_actionEditPeople_triggered ();
		void on_actionEditLaunchMethods_triggered ();
		void on_actionRefreshAll_triggered ();

		// Menu: Debug
		void on_actionSegfault_triggered () { *(int *)NULL = 0; } // For testing the automatic restart mechanism
		//		void on_actionPingServer_triggered ();
		void on_actionTest_triggered ();

		// Menu: Demosystem
		void on_actionWebinterface_triggered () { (void)system ("firefox http://localhost/ &"); }

		// Menu: Help
		void on_actionInfo_triggered ();
		void on_actionNetworkDiagnostics_triggered ();

		// Flight Table
		void on_flightTable_customContextMenuRequested (const QPoint &pos);
		void on_flightTable_activated (const QModelIndex &index);
		void flightTable_buttonClicked (QPersistentModelIndex proxyIndex); // TODO const &, remove dependency
		void flightTable_horizontalHeader_sectionClicked (int index) { sortByColumn (index); }

		// Not connected pane
		void on_connectButton_clicked () { on_actionConnect_triggered (); }

		// Flight manipulation
		bool checkPlaneFlying (dbId id, const QString &description);
		bool checkPersonFlying (dbId id, const QString &description);
		void departFlight (dbId id);
		void landFlight (dbId id);
		void landTowflight (dbId id);

		// Plugins
		void weatherWidget_doubleClicked ();

		// Timers
		void timeTimer_timeout ();

		// Database
		void cacheChanged (DbEvent event);
		void executingQuery (Query);

		void logMessage (QString message);

		void flightListChanged ();

	private:
		Ui::MainWindowClass ui;
		bool oldLogVisible;

		DbManager dbManager;
		Cache &cache;

		QDate displayDate;
		dbId preselectedLaunchMethod;

		QPointer<FlightWindow> createFlightWindow;
		QPointer<FlightWindow> editFlightWindow;

		QList<ShellPlugin *> infoPlugins;
		WeatherWidget *weatherWidget;
		ShellPlugin *weatherPlugin;
		QPointer<WeatherDialog> weatherDialog;

		EntityList<Flight> *flightList;
		FlightProxyList *proxyList;
		FlightModel *flightModel;
		ObjectListModel<Flight> *flightListModel;
		FlightSortFilterProxyModel *proxyModel;

		// The context menu is a property of the class rather than a local
		// variable because it has to persist after the method opening it
		// returns.
		QMenu *contextMenu;

		Qt::SortOrder sortOrder;
		int sortColumn;

		bool databaseActionsEnabled;
		QString databaseOkText;

		/** Whether the font was set explicitly */
		bool fontSet;

};

#endif // MAINWINDOW_H
