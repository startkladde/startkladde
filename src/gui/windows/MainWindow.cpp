#include "MainWindow.h"

#include <QInputDialog>
#include <QPicture>
#include <QFontDialog>
#include <QToolBar>

#include "kvkbd.xpm"
#include "logo.xpm"
#include "src/config/Options.h"
#include "src/db/adminFunctions.h"
#include "src/db/DbEvent.h"
#include "src/db/Database.h"
#include "src/gui/widgets/AcpiWidget.h"
#include "src/gui/widgets/FlightTable.h"
#include "src/gui/windows/DateWindow.h"
#include "src/gui/windows/FlightWindow.h"
#include "src/gui/windows/SplashScreen.h"
#include "src/gui/windows/StatisticsWindow.h"
#include "src/gui/windows/EntityListWindow.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/time/timeFunctions.h"

// UI
MainWindow::MainWindow (QWidget *parent, Database *_db, QList<ShellPlugin> *_plugins, Qt::WindowFlags f)
:
	QMainWindow (parent, f)
/*
 * Initialize the main program window
 * Parameters:
 *   - parent, name, f: See QMainWindow.
 *   - debug: Enable the 'debug' menu.
 *   - title: The title of the window.
 *   - _db: The database connection to be used.
 *   XXX
 */
{
	// Splash screen
	ss = new ::SplashScreen (this, logo);
	ss->show_splash ();
	qApp->processEvents ();


	// Variablen initialisieren
	// TODO: nach UTC einstellen
	// TODO: das in funktion
	anzeigedatum = QDate::currentDate ();
	display_new_flight_date = true;
	always_use_current_date = true;
	db_state = ds_uninitialized;
	startup_complete = false;

	context_row = -1;
	old_row = -1;

	db = _db;

	weatherDialog = NULL;


	initActions ();
	initMenu ();
	initContextMenu ();
	initToolbar ();

	// Splitter
	split_main = new QSplitter (Qt::Vertical, this, "split_main");
	setCentralWidget (split_main);


	// Hauptrahmen
	main_frame = new QFrame (split_main, "main_frame");
	main_layout = new QVBoxLayout (main_frame, window_margin, -1, "main_layout");


	// Infoframe

	// Create and setup the info frame. The info frame ist where the info
	// labels are located, excluding the weather graphics.
	info_frame = new QFrame (main_frame, "info_frame");
	info_frame->setFrameStyle (QFrame::Panel | QFrame::Raised);
	info_frame->setLineWidth (1);

	if (!opts.weather_plugin.isEmpty ())
	{
		// Create and setup the weather widget. The weather widget is located to
		// the right of the info frame.
		weather = new WeatherWidget (main_frame, "weather");
		weather->setFixedHeight (opts.weather_height);
		weather->setFixedWidth (opts.weather_height); // Wird bei Laden eines Bildes angepasst
		weather->setText ("[Regenradar]");
		weather->setFrameStyle (QFrame::Panel | QFrame::Raised);
		weather->setLineWidth (1);

		// Create and setup the weather plugin and connect it to the weather widget
		weather_plugin = new ShellPlugin ("Wetter", opts.weather_plugin, opts.weather_interval); // Initialize to given values
		QObject::connect (weather_plugin, SIGNAL (lineRead (QString)), weather, SLOT (inputLine (QString)));
		// TODO neu laden aus Kontextmenu
		QObject::connect (weather_plugin, SIGNAL (pluginNotFound ()), weather, SLOT (pluginNotFound ()));
		QObject::connect (weather, SIGNAL (doubleClicked ()), this, SLOT (openWeatherDialog ()));
	}
	else
	{
		weather = NULL;
	}

	// Lay out the info frame and the weather plugin
	QHBoxLayout *top_layout = new QHBoxLayout (main_layout);
	top_layout->addWidget (info_frame, 0);
	if (weather) top_layout->addWidget (weather);

	// Lay out the info section and the plugins section within the info frame
	QGridLayout *info_frame_layout = new QGridLayout (info_frame, 1, 2, window_margin, -1, "info_frame_layout");
	QGridLayout *info_layout = new QGridLayout (info_frame_layout, 1, 3, -1, "info_layout");
	// Use a space of 0 for the plugin labels layout because RTF labels spacing is different
	QGridLayout *plugin_layout = new QGridLayout (info_frame_layout, 1, 3, 0, "plugin_layout");

	// This setting gives the plugins more space while still leaving some space
	// between the info and the plugins.
	// The disired behavior would be: info and plugins take up the same amount
	// of space. If the plugin values are too large, the info area is made
	// smaller in favor of the plugin are. If the info area cannot shrink any
	// more, but the plugin area is still too small, the plugin values are
	// wrapped. Never, under no circumstances, is the window resized to make
	// space for the plugins, when something else is possible.
	info_frame_layout->setColStretch (0, 1);
	info_frame_layout->setColStretch (1, 2);
	info_layout->setColStretch (0, 0);
	info_layout->setColStretch (1, 1);
	info_layout->setColStretch (2, 0);
	plugin_layout->setColStretch (0, 0);
	plugin_layout->setColStretch (1, 1);
	plugin_layout->setColStretch (2, 0);

	// Info labels
	for (int i = 0; i < num_info_labels; i++)
	{
		if (i == idx_info_acpi)
		{
			if (AcpiWidget::valid ())
			{
				lbl_info[i] = new QLabel ("[...]", info_frame, "lbl_info[...]");
				lbl_info_value[i] = new AcpiWidget (info_frame);
			}
			else
			{
				lbl_info[i] = NULL;
				lbl_info_value[i] = NULL;
			}
		}
		else
		{
			lbl_info[i] = new QLabel ("[...]", info_frame, "lbl_info[...]");
			lbl_info_value[i] = new QLabel ("[...]", info_frame, "lbl_info_value[...]");
		}

		// If the label is NULL now, skip the rest of the operations.
		if (!lbl_info[i]) continue;

		info_layout->addWidget (lbl_info[i], i, 0);
		info_layout->addWidget (lbl_info_value[i], i, 1);

		if (opts.colorful)
		{
			lbl_info[i]->setPaletteBackgroundColor (QColor (0, 255, 127));
			lbl_info_value[i]->setPaletteBackgroundColor (QColor (0, 127, 255));
		}
		info_layout->setRowStretch (i, 0);
	}
	info_layout->setRowStretch (num_info_labels, 1);

	lbl_info[idx_info_fluege_luft]->setText ("Fl�ge in der Luft: ");
	lbl_info[idx_info_fluege_gesamt]->setText ("Fl�ge gesamt: ");
	lbl_info[idx_info_anzeige_datum]->setText ("Anzeigedatum: ");
	lbl_info[idx_info_utc]->setText ("UTC: ");
	lbl_info[idx_info_time]->setText ("Lokalzeit: ");
	lbl_info[idx_info_datenbankverbindung]->setText ("Datenbankverbindung: ");
	if (AcpiWidget::valid ()) lbl_info[idx_info_acpi]->setText ("Stromversorgung:");

	// Plugins
	plugins = _plugins;
	int row = 0;
	QMutableListIterator<ShellPlugin> it (*plugins);
	while (it.hasNext ())
	{
		ShellPlugin &plugin=it.next ();

		SkLabel *lbl_caption = new SkLabel ("[...]", info_frame, "lbl_caption[...]");
		SkLabel *lbl_value = new SkLabel ("[...]", info_frame, "lbl_value[...]");

		lbl_value->setAlignment (Qt::WordBreak);
		if (plugin.get_rich_text ())
		{
			lbl_caption->setTextFormat (Qt::RichText);
			lbl_caption->setText ("<nobr>" + plugin.get_caption () + "</nobr>");
			lbl_value->setTextFormat (Qt::RichText);
		}
		else
		{
			lbl_caption->setTextFormat (Qt::PlainText);
			lbl_value->setTextFormat (Qt::PlainText);
			lbl_caption->setText (plugin.get_caption ());
		}

		plugin.set_caption_display (lbl_caption);
		plugin.set_value_display (lbl_value);

		plugin_layout->addWidget (lbl_caption, row, 0, Qt::AlignTop);
		plugin_layout->addWidget (lbl_value, row, 1, Qt::AlignTop);
		row++;

		if (opts.colorful)
		{
			lbl_caption->setPaletteBackgroundColor (QColor (255, 63, 127));
			lbl_value->setPaletteBackgroundColor (QColor (255, 255, 127));
		}
		plugin_layout->setRowStretch (row, 0);

		QObject::connect (lbl_caption, SIGNAL (clicked ()), &plugin, SLOT (restart ()));
		QObject::connect (lbl_value, SIGNAL (clicked ()), &plugin, SLOT (restart ()));
	}
	plugin_layout->setRowStretch (row, 1);

	update_info ();


	// Flugtabelle
	tbl_fluege = new FlightTable (db, main_frame);
	main_layout->addWidget (tbl_fluege, 1);
	tbl_fluege->set_anzeigedatum (anzeigedatum);


	// Log
	display_log = false;
	log = new QTextEdit (split_main, "log");
	// TODO better call the function that updates the check mark and
	// hides/displays the log according to the variable.
	update_checks ();
	log->hide ();
	log->setTextFormat (LogText);
	log->document ()->setMaximumBlockCount (1000); // TODO check this works

	connect (db, SIGNAL (executing_query (QString *)), this, SLOT (log_message (QString *)));


	// Timerzeug
	timer_status = new QTimer (this);
	QObject::connect (timer_status, SIGNAL (timeout ()), this, SLOT (update_time ()));
	timer_status->start (1000, false);

	timer_db = new QTimer (this);
	QObject::connect (timer_db, SIGNAL (timeout ()), this, SLOT (slot_timer_db ()));

	update_time ();


	// Sonstige Fensterchen
	// Warum WStyle_StaysOnTop bedeutet, dass es gerade nicht OnTopStays, ist
	// mir ein R�tsel. Jedensfalls ist das anscheinend nur bei nicht-modalen
	// Fensterchen so.
	// Update: Allerdings scheint es nicht zu funktionieren: die Fensterchen
	// [flugzeug|personen]_liste bleiben *immer* im Vordergrund. H�h?
	//	flug_editor=new FlightWindow (this, NULL, flugeditor_modal, Qt::WStyle_Customize | Qt::WStyle_StaysOnTop, ss);
	//	flugzeug_liste=new EntityListWindow (st_plane, this, "flugzeug_liste", false, Qt::WStyle_Customize, ss);
	//	personen_liste=new EntityListWindow (st_person, this, "personen_liste", false, Qt::WStyle_Customize, ss);
	flug_editor = new FlightWindow (this, db, NULL, flugeditor_modal, 0, ss);
	flugzeug_liste = new EntityListWindow (st_plane, this, db, "flugzeug_liste", false, 0, ss);
	personen_liste = new EntityListWindow (st_person, this, db, "personen_liste", false, 0, ss);


	tbl_fluege->setContextMenuPolicy (Qt::CustomContextMenu);

	// Signale verbinden
	QObject::connect (tbl_fluege, SIGNAL (cellDoubleClicked (int, int)), this,
			SLOT (slot_table_double_click (int, int)));
	QObject::connect (tbl_fluege, SIGNAL (key (int)), this, SLOT (slot_table_key (int)));
	QObject::connect (tbl_fluege, SIGNAL (customContextMenuRequested(const QPoint&)), this,
			SLOT(slot_table_context (const QPoint&)));

	QObject::connect (tbl_fluege, SIGNAL (signal_button_start (db_id)), this, SLOT (slot_tbut_start (db_id)));
	QObject::connect (tbl_fluege, SIGNAL (signal_button_landung (db_id)), this, SLOT (slot_tbut_landung (db_id)));
	QObject::connect (tbl_fluege, SIGNAL (signal_button_schlepplandung (db_id)), this,
			SLOT (slot_tbut_schlepplandung (db_id)));

	// Datenbank�ndernde Objekte verbinden
	dbase_connect (flug_editor);
	dbase_connect (flugzeug_liste);
	dbase_connect (personen_liste);

	// Wir sind "db_change hub", also umsetzer von db_change auf db_update
	// Nicht mit dem Signal verbinden, das gibt sonst Schleifen
	// (slot_db_update db_update)
	QObject::connect (this, SIGNAL (db_change (DbEvent *)), this, SLOT (slot_db_update (DbEvent *)));

	QObject *status_dialog = ss;
	// TODO code duplication with SkDialog
	if (status_dialog)
	{
		QObject::connect (this, SIGNAL (status (QString)), status_dialog, SLOT (set_status (QString)));
		QObject::connect (this, SIGNAL (progress (int, int)), status_dialog, SLOT (set_progress (int, int)));
		QObject::connect (this, SIGNAL (long_operation_start ()), status_dialog, SLOT (show_splash ()));
		QObject::connect (this, SIGNAL (long_operation_end ()), status_dialog, SLOT (hide_splash ()));
	}


	connect (qApp, SIGNAL(lastWindowClosed()), this, SLOT(slot_close()));
	connect (actionSetTime, SIGNAL(activated()), this, SLOT(slot_setdate()));
	connect (actionQuit, SIGNAL(activated()), this, SLOT(slot_close()));
	connect (actionShutDown, SIGNAL(activated()), this, SLOT(slot_shutdown()));
	connect (toggleKeyboard, SIGNAL(toggled(bool)), this, SLOT(slotToggleKeyboard(bool)));
	connect (actionNewFlight, SIGNAL(activated()), this, SLOT(slot_flight_new()));
	connect (actionStart, SIGNAL(activated()), this, SLOT(slot_table_start()));
	connect (actionLanding, SIGNAL(activated()), this, SLOT(slot_table_land()));
	connect (actionCtxLanding, SIGNAL(activated()), this, SLOT(slot_context_land()));
	connect (actionGoAround, SIGNAL(activated()), this, SLOT(slot_table_zwischenlandung()));
	connect (actionEditFlight, SIGNAL(activated()), this, SLOT(slot_table_edit()));
	connect (actionCopyFlight, SIGNAL(activated()), this, SLOT(slot_table_wiederholen()));
	connect (actionDeleteFlight, SIGNAL(activated()), this, SLOT(slot_table_delete()));
	connect (actionRefreshTable, SIGNAL(activated()), this, SLOT(slot_refresh_table()));
	connect (actionSetFont, SIGNAL(activated()), this, SLOT(slotSetFont()));
	connect (actionIncFont, SIGNAL(activated()), this, SLOT(slotIncFont()));
	connect (actionDecFont, SIGNAL(activated()), this, SLOT(slotDecFont()));
	connect (actionSchleppRef, SIGNAL(activated()), this, SLOT(slot_schleppref_springen ()));
	connect (actionRestartPlugins, SIGNAL(activated()), this, SLOT(restart_all_plugins ()));
	connect (actionSuppressLanded, SIGNAL(toggled(bool)), this, SLOT(slot_menu_ansicht_flug_gelandete (bool)));
	connect (actionShowDeparted, SIGNAL(toggled(bool)), this,
			SLOT(slot_menu_ansicht_flug_weggeflogene_gekommene (bool)));
	connect (actionShowFaulty, SIGNAL(toggled(bool)), this, SLOT(slot_menu_ansicht_flug_fehlerhafte (bool)));
	connect (actionSortTable, SIGNAL(activated()), this, SLOT(slot_tabelle_sortieren()));
	connect (actionSetDate, SIGNAL(activated()), this, SLOT(slot_menu_ansicht_datum_einstellen()));
	connect (actionNewFlightNewDate, SIGNAL(activated()), this, SLOT(slot_menu_ansicht_display_new_flight_date()));
	connect (actionNewFlightCurrDate, SIGNAL(activated()), this, SLOT(slot_menu_ansicht_always_use_current_date()));
	connect (actionPlaneLog, SIGNAL(activated()), this, SLOT(slot_plane_log()));
	connect (actionPilotLog, SIGNAL(activated()), this, SLOT(slot_pilot_log()));
	connect (actionStartart, SIGNAL(activated()), this, SLOT(slot_sastat()));
	connect (actionEditPlanes, SIGNAL(activated()), this, SLOT(slot_flugzeugeditor()));
	connect (actionEditPersons, SIGNAL(activated()), this, SLOT(slot_personeneditor()));
	connect (actionRefreshAll, SIGNAL(activated()), this, SLOT(slot_db_refresh_all()));
	//        connect(actionWriteCSV,       SIGNAL(activated()),   this, SLOT(slot_csv ()));
	connect (actionInfo, SIGNAL(activated()), this, SLOT(slot_info ()));
	connect (actionNetDiag, SIGNAL(activated()), this, SLOT(slot_netztest ()));
	connect (actionDemoWeb, SIGNAL(activated()), this, SLOT(slot_webinterface()));
	connect (actionSegfault, SIGNAL(activated()), this, SLOT (slot_segfault ()));
	connect (actionPing, SIGNAL(activated()), this, SLOT (slot_ping_db ()));
	connect (actionDisplayLog, SIGNAL(activated()), this, SLOT (slot_display_log ()));

	// Datenbank initialieren
	// Initialize the DB as soon as the GUI is initialized.
	//QTimer::singleShot (0, this, SLOT (slot_timer_db ()));


	// Fenstereinstellungen
	setCaption (opts.title);
	//	resize (480, 360);
	//	setMinimumSize (240, 180);

	update_checks ();

	ss->close ();

	startup_complete = true;
	slot_refresh_table ();
	tbl_fluege->show ();
	tbl_fluege->setFocus ();

	QDesktopWidget *desktop = QApplication::desktop ();
	int w = desktop->width ();
	int h = desktop->height ();
	setMaximumSize (w, h);

	showMaximized ();
	// Wenn das fehlt, kann es passieren, dass das Fenster zu klein ist. Das
	// tritt aber nicht immer auf. Manchmal kann man es provozieren, indem man
	// dem Programm eine unbenutzbare Datenbank vorsetzt. Whatever...
	qApp->processEvents ();


	restart_all_plugins ();

	readSettings ();

}

/**
 * initialize actions
 * TODO: remove context actions?
 * we don't use them
 */
void MainWindow::initActions ()
{
	actionSetTime = new QAction ("&Zeit einstellen", this);

	actionQuit = new QAction ("&Beenden", this);
	actionQuit->setShortcut (CTRL + Key_Q);

	actionShutDown = new QAction ("&Herunterfahren", this);

	/**
	 * wenn die kommandos kvkbd und dcop existieren, wird eine QAction definiert
	 */
	if (system ("which kvkbd") == 0 && system ("which dcop") == 0)
	{
		QIcon icon ((const QPixmap&)QPixmap (kvkbd));
		toggleKeyboard = new QAction (icon, "Tastatur", this);
		toggleKeyboard->setCheckable (true);
		toggleKeyboard->setToolTip ("Ein/Ausblenden der virtuellen Tastatur");
	}
	else
		toggleKeyboard = NULL;

	actionNewFlight = new QAction ("&Neuer Flug...", this);
	actionNewFlight->setShortcut (CTRL + Key_N);

	actionStart = new QAction ("&Starten", this);
	actionStart->setShortcut (CTRL + Key_S);

	actionLanding = new QAction ("&Landen/Beenden", this);
	actionLanding->setShortcut (CTRL + Key_L);

	actionCtxLanding = new QAction ("&Landen/Beenden", this);
	actionCtxLanding->setShortcut (CTRL + Key_L);

	actionGoAround = new QAction ("Zwischenland&ung", this);
	actionGoAround->setShortcut (CTRL + Key_U);

	actionEditFlight = new QAction ("&Editieren...", this);
	actionEditFlight->setShortcut (CTRL + Key_E);

	actionCopyFlight = new QAction ("Wiederh&olen...", this);
	actionCopyFlight->setShortcut (CTRL + Key_O);

	actionDeleteFlight = new QAction ("L�sc&hen...", this);
	actionDeleteFlight->setShortcut (CTRL + Key_H);

	actionRefreshTable = new QAction ("Tabelle aktualisie&ren", this);
	actionRefreshTable->setShortcut (CTRL + Key_R);

	actionSetFont = new QAction ("Schriftart", this);
	actionSetFont->setToolTip ("Schriftart und Gr��e �ndern");

	actionIncFont = new QAction ("+", this);
	actionIncFont->setToolTip ("Schrift vergr��ern");

	actionDecFont = new QAction ("-", this);
	actionDecFont->setToolTip ("Schrift verkleinern");

	actionSchleppRef = new QAction ("Zu schleppref spr&ingen", this);
	actionSchleppRef->setShortcut (CTRL + Key_I);

	actionRestartPlugins = new QAction ("&Plugins neu starten", this);
	actionRestartPlugins->setShortcut (CTRL + Key_P);

	actionSuppressLanded = new QAction ("Gelandete/Weggeflogene Fl�ge ausb&lenden", this);
	actionSuppressLanded->setShortcut (CTRL + Key_A);
	actionSuppressLanded->setCheckable (true);

	actionShowDeparted = new QAction ("&Wegfliegende/Kommende Fl�ge immer anzeigen", this);
	actionShowDeparted->setCheckable (true);

	actionShowFaulty = new QAction ("&Fehlerhafte immer anzeigen", this);
	actionShowFaulty->setCheckable (true);

	actionSortTable = new QAction ("Sor&tieren", this);
	actionSortTable->setShortcut (CTRL + Key_T);

	actionSetDate = new QAction ("Anzeigedatum &einstellen...", this);

	actionNewFlightNewDate = new QAction ("Anzeigedatum bei neuem Flug anpas&sen", this);

	actionNewFlightCurrDate = new QAction ("F�r neue Fl�ge immer &aktuelles Datum verwenden", this);

	actionPlaneLog = new QAction ("&Bordbuch", this);
	actionPlaneLog->setShortcut (CTRL + Key_B);

	actionPilotLog = new QAction ("&Flugbuch", this);
	actionPilotLog->setShortcut (CTRL + Key_F);

	actionStartart = new QAction ("Startartstatisti&k", this);
	actionStartart->setShortcut (CTRL + Key_K);

	actionEditPlanes = new QAction ("Flugzeu&ge editieren", this);
	actionEditPlanes->setShortcut (CTRL + Key_G);

	actionEditPersons = new QAction ("&Personen editieren", this);
	actionEditPersons->setShortcut (CTRL + Key_P);

	actionRefreshAll = new QAction ("Alle &aktualisieren", this);

	//        actionWriteCSV = new QAction ("&CSV schreiben", this);

	actionInfo = new QAction ("&Info", this);

	actionNetDiag = new QAction ("&Netzwerkdiagnose", this);
	actionNetDiag->setEnabled (!opts.diag_cmd.isEmpty ());

	actionSegfault = new QAction ("&Segfault", this);

	actionPing = new QAction ("&Ping server", this);

	actionDisplayLog = new QAction ("&Log anzeigen", this);
	actionDisplayLog->setCheckable (true);

	if (opts.demosystem)
		actionDemoWeb = new QAction ("&Webinterface", this);
	else
		actionDemoWeb = NULL;
}

/**
 * initialize main menu
 */
void MainWindow::initMenu ()
{
	menu_programm = menuBar ()->addMenu ("&Programm");
	menu_flug = menuBar ()->addMenu ("&Flug");
	menu_ansicht = menuBar ()->addMenu ("&Ansicht");
	menu_statistik = menuBar ()->addMenu ("&Statistik");
	menu_datenbank = menuBar ()->addMenu ("&Datenbank");
	if (opts.debug)
		menu_debug = menuBar ()->addMenu ("De&bug");
	else
		menu_debug = NULL;
	menu_hilfe = menuBar ()->addMenu ("&Hilfe");
	if (opts.demosystem)
		menu_demosystem = menuBar ()->addMenu ("D&emosystem");
	else
		menu_demosystem = NULL;

	menu_programm->setTearOffEnabled (true);
	menu_programm->addAction (actionSetTime);
	menu_programm->addSeparator ();
	menu_programm->addAction (actionQuit);
	menu_programm->addAction (actionShutDown);

	menu_flug->setTearOffEnabled (true);
	menu_flug->addAction (actionNewFlight);
	menu_flug->addSeparator ();
	menu_flug->addAction (actionStart);
	// TODO hier k�nnte man das jeweils richtige reinschreiben, abh�ngig vom Flug
	menu_flug->addAction (actionLanding);
	menu_flug->addAction (actionGoAround);
	menu_flug->addSeparator ();
	menu_flug->addAction (actionEditFlight);
	menu_flug->addAction (actionCopyFlight);
	menu_flug->addAction (actionDeleteFlight);

	menu_ansicht->setTearOffEnabled (true);
	menu_ansicht->addAction (actionRefreshTable);
	menu_ansicht_fluege = menu_ansicht->addMenu ("&Fl�ge");
	menu_ansicht_datum = menu_ansicht->addMenu ("&Datum");
	menu_ansicht->addAction (actionSetFont);
	menu_ansicht->addSeparator ();
	menu_ansicht->addAction (actionSchleppRef);
	menu_ansicht->addAction (actionRestartPlugins);

	menu_ansicht_fluege->setTearOffEnabled (true);
	menu_ansicht_fluege->addAction (actionSuppressLanded);
	menu_ansicht_fluege->addAction (actionShowDeparted);
	menu_ansicht_fluege->addAction (actionShowFaulty);
	menu_ansicht_fluege->addAction (actionSortTable);

	menu_ansicht_datum->setTearOffEnabled (true);
	menu_ansicht_datum->addAction (actionSetDate);
	menu_ansicht_datum->addAction (actionNewFlightNewDate);
	menu_ansicht_datum->addAction (actionNewFlightCurrDate);

	menu_statistik->setTearOffEnabled (true);
	menu_statistik->addAction (actionPlaneLog);
	menu_statistik->addAction (actionPilotLog);
	menu_statistik->addAction (actionStartart);

	menu_datenbank->setTearOffEnabled (true);
	menu_datenbank->addAction (actionEditPlanes);
	menu_datenbank->addAction (actionEditPersons);
	menu_datenbank->addSeparator ();
	menu_datenbank->addAction (actionRefreshAll);
	//	menu_datenbank->addSeparator ();
	//	menu_datenbank->addAction (actionWriteCSV);

	menu_hilfe->setTearOffEnabled (true);
	menu_hilfe->addAction (actionInfo);
	menu_hilfe->addAction (actionNetDiag);

	if (menu_debug)
	{
		menu_debug->setTearOffEnabled (true);
		menu_debug->addAction (actionSegfault);
		menu_debug->addAction (actionPing);
		menu_debug->addAction (actionDisplayLog);

		//menu_debug->insertItem ("&Segfault", this, SLOT (slot_segfault ()));
		//menu_debug->insertItem ("&Ping server", this, SLOT (slot_ping_db ()));
		//menu_debug->insertItem ("&Log anzeigen", this, SLOT (slot_display_log ()), 0, id_mnu_debug_display_log);
	}

	if (menu_demosystem) menu_demosystem->addAction (actionDemoWeb);

}

/**
 * initialize context menu
 * we use the normal actions, no special context actions.
 * this seems to work
 */
void MainWindow::initContextMenu ()
{
	menu_ctx_flug = new QMenu (this);
	menu_ctx_flug->addAction (actionNewFlight);
	menu_ctx_flug->addSeparator ();

	menu_ctx_flug->addAction (actionStart);
	menu_ctx_flug->addAction (actionLanding);
	menu_ctx_flug->addAction (actionGoAround);
	menu_ctx_flug->addSeparator ();
	menu_ctx_flug->addAction (actionEditFlight);
	menu_ctx_flug->addAction (actionCopyFlight);
	menu_ctx_flug->addAction (actionDeleteFlight);

	// Kontextmen� f�r leere Zeilen
	menu_ctx_emptyrow = new QMenu (this);
	menu_ctx_emptyrow->addAction (actionNewFlight);
}

/**
 * initialize toobar
 */
void MainWindow::initToolbar ()
{
	toolBar = new QToolBar (this);
	addToolBar (Qt::TopToolBarArea, toolBar);

	if (toggleKeyboard) toggleKeyboard->addTo (toolBar);
	actionNewFlight->addTo (toolBar);
	actionCopyFlight->addTo (toolBar);
	actionSuppressLanded->addTo (toolBar);
	actionIncFont->addTo (toolBar);
	actionDecFont->addTo (toolBar);
}

MainWindow::~MainWindow ()
/*
 * Clean up the main program window variables
 */
{
	//	// Tabellenspaltenbreitenausgabe
	//	QHeader *table_header=tbl_fluege->horizontalHeader ();
	//	for (int i=0; i<tabellenspalten; i++)
	//	{
	//		printf ("%d\n", table_header->sectionRect (i).width ());
	//	}

	if (plugins)
	{
		QMutableListIterator<ShellPlugin> plugin (*plugins);
		while (plugin.hasNext())
		{
			plugin.peekNext().terminate();
			sched_yield();
			std::cout << "Terminating plugin " << plugin.peekNext().get_caption () << std::endl;
			plugin.next ();
		}
	}

}

void MainWindow::keyPressEvent (QKeyEvent *e)
/*
 * A key was pressed. Process the key event.
 * Parameters:
 *   - e: See ::keyPressEvent.
 */
{
	// Ganz �bler Pfusch, weil das SplashScreen nicht den Focus erh�lt, wenn �ber
	// Hilfe->Info aufgerufen.
	ss->try_close ();

	switch (e->key ())
	{
		case Qt::Key_F2:
			slot_flight_new ();
			break;
		case Qt::Key_F3:
			slot_table_wiederholen ();
			break;
		case Qt::Key_F4:
			slot_table_edit ();
			break;

		case Qt::Key_F5:
			slot_table_start ();
			break;
		case Qt::Key_F6:
			slot_table_land ();
			break;
		case Qt::Key_F7:
			slot_table_zwischenlandung ();
			break;
		case Qt::Key_F8:
			slot_table_delete ();
			break;

			//		case Qt::Key_F9: slot_tabelle_sortieren (); break;	// In SkTable gemacht
			//		case Qt::Key_F10: slot_close (); break;
		case Qt::Key_F11:
			slot_menu_ansicht_flug_gelandete (true);
			break;
		case Qt::Key_F12:
			slot_refresh_table ();
			break;
		default:
			e->ignore ();
			break;
	}

	QMainWindow::keyPressEvent (e);
}

void MainWindow::simulate_key (int key)
/*
 * Act as if a key had been pressed on the main window.
 * Parameters:
 *   - key: the key to simulate.
 */
{
	QKeyEvent e (QEvent::KeyPress, key, 0, 0);
	keyPressEvent (&e);
}

// DB
void MainWindow::slot_db_update (DbEvent *event)
/*
 * (db_change) Something happened at the database. Check if we are
 * interested and accordingly.
 * Parameters:
 *   - event: The event that happened.
 */
{
	event->dump ();
	if (event->type == det_refresh) slot_refresh_table ();

	if (event->table == db_flug)
	{
		if (event->type == det_add)
		{
			// TODO: das in funktion
			if (display_new_flight_date)
			{
				Flight fl;
				int ret = db->get_flight (&fl, event->id);
				if (ret == db_ok && fl.happened ()) set_anzeigedatum (fl.effdatum ());
			}

			Flight fl;
			if (event->id != 0 && db->get_flight (&fl, event->id) == db_ok)
			{
				//TODO das immer
				if (fl.effdatum () == anzeigedatum || fl.vorbereitet ()) tbl_fluege->update_flight (event->id, &fl);

				int fl_row = tbl_fluege->row_from_id (event->id);
				int fl_col = tbl_fluege->currentColumn ();
				tbl_fluege->setCurrentCell (fl_row, fl_col);
			}
		}
		if (event->type == det_change)
		{
			Flight fl;
			int ret = db->get_flight (&fl, event->id);
			if (ret == db_ok) tbl_fluege->update_flight (event->id, &fl);
		}
		else if (event->type == det_delete)
		{
			tbl_fluege->remove_flight (event->id);
		}

		menu_enables ();
		update_info ();
	}
	else
	{
		// Wenn etwas anderes als die Flugdatenbank ge�ndert wurde, Tabelle
		// aktualisieren, da sich einiges ge�ndert haben kann.
		// TODO: Wir wissen, welche ID, daher k�nnen wir auch selektiv
		// update_flight()s machen.
		if (event->type == det_change) slot_refresh_table ();
		if (event->type == det_delete) slot_refresh_table ();
	}

	emit db_update (event);
}

void MainWindow::dbase_connect (QObject *ob)
/*
 * (db_change) Connect the signals used for the db_change mechanism with a
 * contained object. See SkDialog.cpp.
 * Parameters:
 *   - ob: The object to connect with.
 * This code is the same as in SkDialog. Multiple inheritance would be
 * better here.
 */
{
	// Wenn ein Objekt die Datenbank ge�ndert hat, weiterreichen.
	QObject::connect (ob, SIGNAL (db_change (DbEvent *)), this, SIGNAL (db_change (DbEvent *)));
	// Wenn Datenbank�nderungen �bernommen werden sollen, weiterreichen.
	QObject::connect (this, SIGNAL (db_update (DbEvent *)), ob, SLOT (slot_db_update (DbEvent *)));
}


// Fl�ge
void MainWindow::neuer_flug (Flight *vorlage)
/*
 * Open the dialog for creating a new flight.
 * Parameters:
 *   - vorlage: a template for a flight (for the repeating function), or
 *     NULL for an empty flight.
 * TODO: is *vorlage destroyed?
 */
{
	// Flugdialog auf�hren
	if (vorlage)
		flug_editor->duplicate_flight (vorlage);
	else
		flug_editor->create_flight (always_use_current_date ? NULL : &anzeigedatum);
}

void MainWindow::edit_flight (Flight *f)
/*
 * Open the dialog for editing a flight.
 * Parameters:
 *   - f: points to the flight data to be editied.
 * TODO: is *f destroyed? is the database updated?
 */
{
	// Flugdialog ausf�hren
	// TODO: was passiert, wenn f==NULL?
	flug_editor->edit_flight (f);
	// TODO error handling?
}

db_id MainWindow::get_flight_id (int row)
/*
 * Find the ID of the flight contained in a given row of the main Table.
 * Parameters:
 *   - row: the row containing the flight.
 * Return value:
 *   - 0 on error or if the row contains no flight.
 *   - the ID of the flight else.
 */
{
	if (tbl_fluege->row_is_flight (row))
	{
		db_id id = tbl_fluege->id_from_row (row);
		return id;
	}
	else
	{
		return 0;
	}
}

void MainWindow::manipulate_flight (db_id id, flight_manipulation action, db_id sref)
/*
 * Do a change to a flight or an airtow identified by the database ID of the
 * flight.
 * Parameters:
 *   - id: the database ID of the flight to manipulate if the flight is to
 *     be manipulated, 0 else.
 *   - action: what to do to the flight.
 *   - sref: the database ID of the flight to manipulate if the airtow
 *     is to be manipulated.
 */
/*
 * TODO: multiple id-�bergable ist doof, besser enum { mt_flug,
 * mt_schleppflug } manipulation_target einf�hren.
 * TODO: hier den ganzen Kram wie anzeigedatum==heute und flug schon
 * gelandet pr�fen, damit man die Men�deaktivierung wegmachen kann.
 * Au�erdem kann man dann hier melden, warum das nicht geht.
 */
{
	bool changed = false;
	db_id target_id = id;

	// Falls per sref aufgerufen wurde, betreffen die Operationen den
	// Schleppflug, werden aber auf dem eigentlichen Flug ausgef�hrt, da der
	// Schleppflug nicht als eigener Flug gef�hrt wird.
	// Einige Aktionen werden bei einem Schleppflug auch gar nicht ausgef�hrt.
	if (id_invalid (id))
	{
		// Aufruf per sref.
		target_id = sref;
		bool abort = false;
		switch (action)
		{
			case fm_start:
				// Flug starten
				break;
			case fm_land:
				// Schleppflugzeug des Flugs landen
				action = fm_land_schlepp;
				break;
			case fm_land_schlepp:
				log_error ("Schlepplandung f�r Schleppflug in sk_win_main::manipulate_flight ()");
				// Trotzdem machen
				break;
			case fm_zwischenlandung:
				// Zwischenlandung des Segelflugs
				//				show_warning ("Zwischenlandung f�r Schleppflug nicht m�glich.", this);
				//				abort=true;
				break;
			case fm_edit:
				// Flug wird editiert.
				break;
			case fm_delete:
				// Flug wird gel�scht
				break;
			case fm_wiederholen:
				show_warning ("Schleppfl�ge k�nnen nicht wiederholt werden.", this);
				abort = true;
				break;
			default:
				log_error ("Unhandled action for Schleppflug in sk_win_main::manipulate_flight ()");
				abort = true;
				break;
		}
		if (abort) return;
	}

	if (action == fm_delete)
	{
		QString query_string;
		if (id > 0)
			query_string = "Flug wirklich l�schen?";
		else
			query_string
					= "Dies ist ein Schleppflug. Soll der dazu geh�rige\ngeschleppte Flug wirklich gel�scht werden?";

		int ret = QMessageBox::information (this, "Wirklich l�schen?", query_string, "&Ja", "&Nein", QString::null, 0,
				1);

		if (ret == 0)
		{
			int db_result = db->delete_flight (target_id);
			if (db_result == db_ok)
			{
				DbEvent event (det_delete, db_flug, target_id);
				emit db_change (&event);
			}
			else
			{
				QMessageBox::warning (this, "Fehler beim L�schen", "Fehler beim L�schen: " +
						db->db_error_description (db_result), QMessageBox::Ok, QMessageBox::NoButton);
			}
		}
	}
	else
	{
		/* versuch, gelandete flüge für 60 sekunden in der liste zu lassen;
		 * klappt so noch nicht.
		 qDebug() << "starttimer " << endl;
		 int row=tbl_fluege->row_from_id (id);
		 SkTableItem* item0 = (SkTableItem*)tbl_fluege->item (row, 0);
		 if (item0)
		 item0->startTimer (60000);
		 */

		if (action == fm_start && display_new_flight_date) set_anzeigedatum (QDate::currentDate ());

		// Flug aus Datenbank lesen
		Flight f;
		if (db->get_flight (&f, target_id) < 0)
		{
			log_error ("Datenbankanfrage fehlgeschlagen in sk_win_main::manipulate_flight ()");
		}
		else
		{
			// TODO error handling
			LaunchType sa;
			db->get_startart (&sa, f.startart);

			bool action_ok = true;
			Plane fz;
			if (f.flugzeug > 0 && db->get_plane (&fz, f.flugzeug) < 0)
			{
				log_error ("Flugzeugdatenbankanfrage fehlgeschlagen in sk_win_main::manipulate_flight ()");
				// Trotzdem weiter machen, an dem fehlenden Flugzeug soll der Flug nicht scheitern
				// TODO: wird hier immer gepr�ft, ob das FZ tats�chlich g�ltig ist?
			}
			else
			{
				if (f.flugzeug > 0 && action == fm_zwischenlandung && fz.category == Plane::categoryGlider
						&& !sa.is_airtow ())
				{
					show_warning ("Ein Segelflugzeug kann keine Zwischenlandung machen.", this);
					action_ok = false;
				}

			}

			if (action_ok)
			{
				switch (action)
				{
					case fm_start:
					{
						bool starten = true;

						// TODO currentDateTime ersetzen durch Time
						// TODO unified fehlerchecking
						// TODO das hier ist codeduplikation mit FlightWindow
						// TODO hier auch schleppmaschine (bzw. besser nicht
						// hier sondern an einheitlicher Stelle)
						Time ct;
						ct.set_current ();
						if (starten && !id_invalid (f.pilot) && !id_invalid (db->person_flying (f.pilot, &ct)))
						{
							Person p;
							db->get_person (&p, f.pilot);
							QString msg = "Laut Datenbank fliegt der " + f.pilot_bezeichnung () + " \"" + p.text_name ()
									+ "\" noch.\n";
							if (!check_message (this, msg)) starten = false;
						}
						if (starten && !id_invalid (f.begleiter) && !id_invalid (db->person_flying (f.begleiter, &ct)))
						{
							Person p;
							db->get_person (&p, f.begleiter);
							QString msg = "Laut Datenbank fliegt der " + f.begleiter_bezeichnung () + " \""
									+ p.text_name () + "\" noch.\n";
							if (!check_message (this, msg)) starten = false;
						}
						if (starten && !id_invalid (f.flugzeug) && !id_invalid (db->plane_flying (f.flugzeug, &ct)))
						{
							Plane fz;
							db->get_plane (&fz, f.flugzeug);
							QString msg = "Laut Datenbank fliegt das Flugzeug \"" + fz.registration + "\" noch.\n";
							if (!check_message (this, msg)) starten = false;
						}

						if (starten && f.starten (false, true)) changed = true;
						break;
					}
					case fm_land:
						if (f.landen (false, true)) changed = true;
						break;
					case fm_land_schlepp:
						if (f.schlepp_landen (false, true)) changed = true;
						break;
					case fm_zwischenlandung:
						if (f.zwischenlandung (false, true)) changed = true;
						break;
						// Bei edit: changed=false, weil der Flugeditor die
						// �nderungen eintr�gt und db_changed ausl�st.
					case fm_edit:
						edit_flight (&f);
						changed = false;
						break; // Kein changed: db_changed wird vom Editor ausgel�st
					case fm_wiederholen:
						neuer_flug (&f);
						changed = false;
						break;
					default:
						log_error ("Unhandled action in sk_win_main::manipulate_flight ()");
						break;
				}
			}
			if (changed)
			{
				// TODO Fehlerbehandlung
				db->write_flight (&f);

				DbEvent event (det_change, db_flug, target_id);
				emit db_change (&event);
			}
		}
	}

}

void MainWindow::manipulate_flight_by_row (int row, flight_manipulation action)
/*
 * Do a change to a flight or an airtow identified by its row in the main
 * Table.
 * Parameters:
 *   - row: the row of the the flight or the airtow to be manipulated.
 *   - action: what to do to the flight.
 */
{
	int use_row = row >= 0 ? row : tbl_fluege->currentRow ();
	bool ist_schlepp = false;

	if (!tbl_fluege->row_is_flight (tbl_fluege->currentRow ())) return;

	// ID des Flugs aus der Tabellenzeile lesen
	db_id id = tbl_fluege->id_from_row (use_row);
	if (id_invalid (id))
	{
		// ID ist 0 ==> Schleppflug, ID des geschleppten Flugs lesen
		id = tbl_fluege->schleppref_from_row (use_row);
		ist_schlepp = true;
	}

	if (id_invalid (id))
	{
		// ID ist immer noch 0 ==> Die Tablle enth�lt G�lle.
		log_error ("Tabellenzeile ist kein Flug");
	}
	else
	{
		// Flug aus der Datenbank lesen.
		Flight f;
		if (db->get_flight (&f, id) == db_ok)
		{
			if (ist_schlepp)
				manipulate_flight (0, action, id);
			else
				manipulate_flight (id, action);
		}
		else
		{
			log_error ("Fehler beim Lesen des Flugs");
		}
	}
}

void MainWindow::slot_flight_new ()
/*
 * "New flight" was chosen. Create a new flight.
 */
{
	if (!db_available ()) return;
	neuer_flug ();
}

// Tabelle
void MainWindow::table_activated (int row)
/*
 * Do the default action for Table activation.
 * If a flight is selected, edit it. Else, create a new one.
 * Parameters:
 *   - row: The row which was activated.
 */
{
	// TODO was ist mit table_activated (-)?
	if (!db_available ()) return;
	if (tbl_fluege->row_is_flight (row))
	{
		manipulate_flight_by_row (row, fm_edit);
	}
	else
	{
		slot_flight_new ();
	}
}

/**
 * Refresh all data in the Table.
 */
void MainWindow::slot_refresh_table ()
{
	// If startup is not completed, there might not be a database connection or
	// the Table yet (?, TODO check reason).
	if (!db_available ()) return;
	if (!startup_complete) return;

	emit
	long_operation_start ();
	emit
	status ("Hauptfenster: Flugtabelle aktualisieren...");

	if (db_available ())
	{
		int old_col = tbl_fluege->currentColumn ();
		//int old_row=tbl_fluege->currentRow ();
		//db_id old_id=tbl_fluege->id_from_row (old_row);

		tbl_fluege->hide ();
		tbl_fluege->removeAllRows ();

		// Flüge von heute
		QList<Flight *> flights;
		db->list_flights_date (flights, &anzeigedatum);
		if (anzeigedatum == QDate::currentDate ()) db->list_flights_prepared (flights);

		std::cout << "Die datenbank reicht uns " << flights.count () << " Flyge ryber." << std::endl;
		qSort (flights);
		foreach (Flight *f, flights)
			tbl_fluege->update_flight (f->id, f);

		tbl_fluege->show ();
		//int new_row=tbl_fluege->row_from_id (old_id);
		//if (new_row<0)
		// es ist angenehmer, immer das Ende der Tabelle zu fokussieren
		int new_row = tbl_fluege->rowCount () - 1;
		tbl_fluege->setCurrentCell (new_row, old_col);
		//tbl_fluege->ensureCellVisible(new_row, old_col);

		update_info ();
		// Scheint nicht zu funktionieren, wenn Tabelle leer, wie kriegt man
		// den Focus in diesem Fall auf die Tabelle?
		tbl_fluege->setFocus ();

		foreach (Flight *f, flights) delete f;
	}

	emit long_operation_end ();
}

void MainWindow::slot_table_context (const QPoint &pos)
{
	int row = tbl_fluege->rowAt (pos.y ());

	// Zeile speichern
	context_row = row;

	if (!db_available ()) return;

	if (tbl_fluege->row_is_flight (row))
	{
		// Das ist ein eingetragener Flug, Men� zum Starten/Landen/Editieren/
		// L�schen �ffnen
		bool ist_schlepp = false;
		db_id id = tbl_fluege->id_from_row (row);
		if (id_invalid (id))
		{
			id = tbl_fluege->schleppref_from_row (row);
			ist_schlepp = true;
		}

		Flight f;
		if (db_available ())
		{
			// Datenbank ist OK
			actionNewFlight->setEnabled (true);

			if (db->get_flight (&f, id) >= 0 && f.editierbar)
			{
				if (true/* && anzeigedatum==QDate::currentDate ()*/)
				//         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				//         Was soll der Bl�dsinn? Fl�ge von ande-
				//         ren Tagen sind auch noch editierbar,
				//         wenn sie als editierbar in der
				//         Datenbank stehen!
				{
					// TODO: CODEDUPLIKATION! Auf unified fehlerchecking umstellen!
					// TODO: CODEDUPLIKATION: Das ist der gleiche Code wie in menu_enables (mit einem anderen Men�).
					//menu_ctx_flug->setItemEnabled (id_ctx_starten, starts_here (f.modus) && !f.gestartet);
					actionStart->setEnabled (starts_here (f.modus) && !f.gestartet);
					//menu_ctx_flug->setItemEnabled (id_ctx_editieren, true);
					actionEditFlight->setEnabled (true);
					//menu_ctx_flug->setItemEnabled (id_ctx_loeschen, true);
					actionDeleteFlight->setEnabled (true);

					actionCtxLanding->setEnabled (ist_schlepp ? f.sfz_fliegt () : (f.fliegt () && lands_here (f.modus)));
					//menu_ctx_flug->setItemEnabled (id_ctx_zwischenlandung, f.fliegt ());
					actionGoAround->setEnabled (f.fliegt ());
					//menu_ctx_flug->setItemEnabled (id_ctx_wiederholen, !ist_schlepp);
					actionCopyFlight->setEnabled (!ist_schlepp);
				}
				else
				{
					// TODO Codeduplikation...
					//menu_ctx_flug->setItemEnabled (id_ctx_starten, false);
					actionStart->setEnabled (false);
					//menu_ctx_flug->setItemEnabled (id_ctx_editieren, false);
					actionEditFlight->setEnabled (false);
					//menu_ctx_flug->setItemEnabled (id_ctx_loeschen, false);
					actionDeleteFlight->setEnabled (false);
					actionCtxLanding->setEnabled (false);
					//menu_ctx_flug->setItemEnabled (id_ctx_zwischenlandung, false);
					actionGoAround->setEnabled (false);
					//menu_ctx_flug->setItemEnabled (id_ctx_wiederholen, false);
					actionCopyFlight->setEnabled (false);
				}

				// TODO Bedingung �berpr�fen
				actionCtxLanding->setText ((ist_schlepp && !lands_here (f.modus_sfz)) ? "Sch&lepp beenden" : "&Landen");
			}
			else
			{
				log_error ("Kein Flug aus der Datenbank in sk_win_main:;slot_table_context ()");
			}
		}
		else
		{
			// Datenbank ist nicht OK
			actionNewFlight->setEnabled (false);

			//menu_ctx_flug->setItemEnabled (id_ctx_starten, false);
			actionStart->setEnabled (false);
			//menu_ctx_flug->setItemEnabled (id_ctx_editieren, false);
			actionEditFlight->setEnabled (false);
			//menu_ctx_flug->setItemEnabled (id_ctx_loeschen, false);
			actionDeleteFlight->setEnabled (false);
			actionCtxLanding->setEnabled (false);
			//menu_ctx_flug->setItemEnabled (id_ctx_zwischenlandung, false);
			actionGoAround->setEnabled (false);
			//menu_ctx_flug->setItemEnabled (id_ctx_wiederholen, false);
			actionCopyFlight->setEnabled (false);
		}

		menu_ctx_flug->popup (tbl_fluege->mapToGlobal (pos));
	}
	else
	{
		// Leerer Platz/leere Zeile, Men� zum Anlegen �fnen
		actionNewFlight->setEnabled (db_available ());

		menu_ctx_emptyrow->popup (tbl_fluege->mapToGlobal (pos));
	}
}

void MainWindow::slot_table_double_click (int row, int col)
/*
 * The Table was double-clicked. Process the double click.
 * Parameters:
 *   - row: the row where the Table was double-clicked.
 *   - col: the column where the Table was double-clicked.
 */
{
	(void)col;
	table_activated (row);
}

void MainWindow::slot_table_key (int key)
/*
 * A key was pressed on the main Table. Process the keypress.
 * Parameters:
 *   - key: the key that was pressed.
 */
{

	switch (key)
	{
		case Qt::Key_Return:
		{
			int orow = tbl_fluege->currentRow ();
			int ocol = tbl_fluege->currentColumn ();
			table_activated (tbl_fluege->currentRow ());
			tbl_fluege->setCurrentCell (orow, ocol);
		}
			break;
		case Qt::Key_Insert:
			slot_flight_new ();
			break;
		case Qt::Key_Delete:
			slot_table_delete ();
			break;
		default:
			break;
	}
}

void MainWindow::slot_current_changed ()
/*
 * The current row changed. Do what is needed to reflect the newly selected
 * flight at the user interface.
 */
{
	menu_enables (true);
}

// Flight manipulations
/*
 * These functions get called when the corresponding menu entries are selected.
 */
void MainWindow::slot_table_start ()
{
	if (!db_available ()) return;
	manipulate_flight_by_row (tbl_fluege->currentRow (), fm_start);
}

void MainWindow::slot_table_land ()
{
	if (!db_available ()) return;
	manipulate_flight_by_row (tbl_fluege->currentRow (), fm_land);
}

void MainWindow::slot_table_zwischenlandung ()
{
	if (!db_available ()) return;
	manipulate_flight_by_row (tbl_fluege->currentRow (), fm_zwischenlandung);
}

void MainWindow::slot_table_edit ()
{
	if (!db_available ()) return;
	manipulate_flight_by_row (tbl_fluege->currentRow (), fm_edit);
}

void MainWindow::slot_table_delete ()
{
	if (!db_available ()) return;
	manipulate_flight_by_row (tbl_fluege->currentRow (), fm_delete);
}

void MainWindow::slot_table_wiederholen ()
{
	if (!db_available ()) return;
	manipulate_flight_by_row (tbl_fluege->currentRow (), fm_wiederholen);
}

// Tabellenbuttons
/*
 * These functions get called when the corresponding button in the Table
 * is pressed.
 */
void MainWindow::slot_tbut_start (db_id id)
{
	manipulate_flight (id, fm_start);
}

void MainWindow::slot_tbut_landung (db_id id)
{
	manipulate_flight (id, fm_land);
}

void MainWindow::slot_tbut_schlepplandung (db_id id)
{
	manipulate_flight (id, fm_land_schlepp);
}

// Infoframe
void MainWindow::update_info ()
/*
 * Updates all information labels in the information frame except the time
 * labels.
 */
{
	QString s;
	if (db_available ())
	{
		int flightsCurrent = db->count_flights_current (anzeigedatum);
		int flightsToday = db->count_flights_today (anzeigedatum);

		if (flightsCurrent >= 0)
			lbl_info_value[idx_info_fluege_luft]->setText (s.setNum (flightsCurrent));
		else
			lbl_info_value[idx_info_fluege_luft]->setText (db->db_error_description (flightsCurrent));

		if (flightsToday >= 0)
			lbl_info_value[idx_info_fluege_gesamt]->setText (s.setNum (flightsToday));
		else
			lbl_info_value[idx_info_fluege_gesamt]->setText (db->db_error_description (flightsToday));
	}
	else
	{
		lbl_info_value[idx_info_fluege_luft]->setText ("(Datenbank nicht verf�gbar)");
		lbl_info_value[idx_info_fluege_gesamt]->setText ("(Datenbank nicht verf�gbar)");
	}

	lbl_info_value[idx_info_anzeige_datum]->setText (anzeigedatum.toString ("yyyy-MM-dd"));
}

void MainWindow::update_time ()
/*
 * Updates the time labels in the information frame with the current time.
 */
{
	lbl_info_value[idx_info_utc]->setText (get_current_time_text (tz_utc));
	lbl_info_value[idx_info_time]->setText (get_current_time_text (tz_local));

	tbl_fluege->update_time ();
}

// Kontextmen�
/*
 * These functions are called when a menu entry from the context menu is
 * selected. They use the context_row instead of the current Table row.
 * Question: but the context_row seems always to be identical to the current row?
 */
void MainWindow::slot_context_start ()
{
	manipulate_flight_by_row (context_row, fm_start);
}

void MainWindow::slot_context_land ()
{
	manipulate_flight_by_row (context_row, fm_land);
}

void MainWindow::slot_context_zwischenlandung ()
{
	manipulate_flight_by_row (context_row, fm_zwischenlandung);
}

void MainWindow::slot_context_edit ()
{
	manipulate_flight_by_row (context_row, fm_edit);
}

void MainWindow::slot_context_delete ()
{
	manipulate_flight_by_row (context_row, fm_delete);
}

void MainWindow::slot_context_wiederholen ()
{
	manipulate_flight_by_row (context_row, fm_wiederholen);
}

// Men�s
void MainWindow::menu_enables (bool cell_change)
/*
 * Enables or disables entries in the menus, according to the selected
 * flight and the database connection status.
 * Parameters:
 *   - cell_change: whether the enable update was caused by a cell change.
 *     This means that we have to query the database.
 */
{
	// Zeile speichern
	int row = tbl_fluege->currentRow ();

	bool alife = db_available ();

	actionRefreshTable->setEnabled (alife);
	actionSetDate->setEnabled (alife);

	actionPlaneLog->setEnabled (alife);
	actionPilotLog->setEnabled (alife);
	actionStartart->setEnabled (alife);
	actionEditPlanes->setEnabled (alife);
	actionEditPersons->setEnabled (alife);
	actionRefreshAll->setEnabled (alife);
	//	actionWriteCSV->setEnabled (alife);

	QString sref_springen_text;
	bool sref_springen_enabled;
	db_id id = tbl_fluege->id_from_row (row);
	db_id sref = tbl_fluege->schleppref_from_row (row);

	//bool ist_flug=false;
	bool ist_schlepp = false;

	if (id_invalid (id) && id_invalid (sref))
	{
		// Fehler, kein Flug und kein Schlepp
		sref_springen_text = "Schleppflug";
		sref_springen_enabled = false;
	}
	else if (id_invalid (id))
	{
		// Schlepp
		sref_springen_text = "geschlepptem Flug";
		sref_springen_enabled = true;
		ist_schlepp = true;
	}
	else
	{
		// Flug
		sref_springen_text = "Schleppflug";
		sref_springen_enabled = (tbl_fluege->row_from_sref (id) >= 0);
		ist_schlepp = false;
	}
	actionSchleppRef->setText ("Zu " + sref_springen_text + " spr&ingen");
	actionSchleppRef->setEnabled (sref_springen_enabled);

	if (alife)
	{
		// Datenbankverbindung ist vorhanden
		actionNewFlight->setEnabled (true);

		if (row != old_row || !cell_change)
		{
			if (tbl_fluege->row_is_flight (row)/* && anzeigedatum==QDate::currentDate ()*/)
			//                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			//                                    Was soll der Bl�dsinn? Fl�ge von ande-
			//                                    ren Tagen sind auch noch editierbar,
			//                                    wenn sie als editierbar in der
			//                                    Datenbank stehen!
			{
				// Das ist ein eingetragener Flug, Men� zum Starten/Landen/Editieren/
				// L�schen �ffnen
				db_id id = tbl_fluege->id_from_row (row);
				bool ist_schlepp = false;
				if (id_invalid (id))
				{
					id = tbl_fluege->schleppref_from_row (row);
					ist_schlepp = true;
				}

				// TODO das kann man sich unter Umst�nden sparen, wenn es
				// Geschwindigkeit bringt; ob die Aktionen erlaubt sind,
				// wird sowieso separat gepr�ft, weil die Funktionstasten
				// hier nicht gesperrt werden.
				// Dann aber auch alles auf true setzen.
				// Update: wir unterscheiden auch zwischen Flug und Schlepp,
				// daher sollte man diesen Passus nicht ersatzlos streichen
				// sondern eher den Flug in einer Tabellenzeile speichern.
				// TODO make it so

				Flight f;
				// TODO hier und �berall ist_schlepp und ist_flug verwenden.
				if (id > 0 && db->get_flight (&f, id) >= 0)
				{
					//menu_flug->setItemEnabled (id_ctx_starten, starts_here (f.modus) && !f.gestartet);
					actionStart->setEnabled (starts_here (f.modus) && !f.gestartet);
					//menu_flug->setItemEnabled (id_ctx_editieren, true);
					actionEditFlight->setEnabled (true);
					//menu_flug->setItemEnabled (id_ctx_loeschen, true);
					actionDeleteFlight->setEnabled (true);
					actionCtxLanding->setEnabled (ist_schlepp ? f.sfz_fliegt () : (lands_here (f.modus) && !f.gelandet));
					//menu_flug->setItemEnabled (id_ctx_zwischenlandung, !f.gelandet);
					actionGoAround->setEnabled (!f.gelandet);
					//menu_flug->setItemEnabled (id_ctx_wiederholen, !ist_schlepp);
					actionCopyFlight->setEnabled (!ist_schlepp);
					// TODO codeduplikation Text
					actionCtxLanding->setText ((ist_schlepp && !lands_here (f.modus_sfz)) ? "Sch&lepp beenden"
							: "&Landen");
				}
			}
			else
			{
				//menu_flug->setItemEnabled (id_ctx_starten, false);
				actionStart->setEnabled (false);
				actionCtxLanding->setEnabled (false);
				//menu_flug->setItemEnabled (id_ctx_zwischenlandung, false);
				actionGoAround->setEnabled (false);
				//menu_flug->setItemEnabled (id_ctx_editieren, false);
				actionEditFlight->setEnabled (false);
				//menu_flug->setItemEnabled (id_ctx_wiederholen, false);
				actionCopyFlight->setEnabled (false);
				//menu_flug->setItemEnabled (id_ctx_loeschen, false);
				actionDeleteFlight->setEnabled (false);
			}
		}
	}
	else
	{
		// Datenbankverbindung ist nicht vorhanden
		actionNewFlight->setEnabled (false);

		//menu_flug->setItemEnabled (id_ctx_starten, false);
		actionStart->setEnabled (false);
		actionCtxLanding->setEnabled (false);
		//menu_flug->setItemEnabled (id_ctx_zwischenlandung, false);
		actionGoAround->setEnabled (false);
		//menu_flug->setItemEnabled (id_ctx_editieren, false);
		actionEditFlight->setEnabled (false);
		//menu_flug->setItemEnabled (id_ctx_wiederholen, false);
		actionCopyFlight->setEnabled (false);
		//menu_flug->setItemEnabled (id_ctx_loeschen, false);
		actionDeleteFlight->setEnabled (false);
	}

	old_row = row;
}

//void MainWindow::slot_menu (int i)
//	/*
//	 * Gets called when a menu is opened. Does the necessary enables for the
//	 * menu.
//	 * Parameters:
//	 *   - i: the ID of the menu that was opened.
//	 */
//{
//	if (i==id_mnu_flug)
//	{
//		menu_enables ();
//	}
//}

void MainWindow::update_checks ()
/*
 * Updates the check marks in the menus according to current settings.
 */
{
	// TODO only update the checks which have changed, from where they changed
	actionSuppressLanded->setChecked (tbl_fluege->gelandete_ausblenden);
	actionShowDeparted->setChecked (tbl_fluege->weggeflogene_gekommene_anzeigen);
	actionShowFaulty->setChecked (tbl_fluege->fehlerhafte_immer);
	actionNewFlightNewDate->setChecked (display_new_flight_date);
	actionNewFlightCurrDate->setChecked (always_use_current_date);
	actionDisplayLog->setChecked (display_log);
}

// Men� Programm
void MainWindow::slot_setdate ()
/*
 * Open the time editor to change the date.
 */
{
	DateWindow editor (this, QDateTime::currentDateTime ());
	QObject::connect (&editor, SIGNAL (datetime_accepted (QDateTime)), this, SLOT (setdate (QDateTime)));
	editor.edit_datetime ();
}

void MainWindow::setdate (QDateTime dt)
/*
 * Updates the current system time.
 * Parameters:
 *   - dt: The date to set the system time to.
 */
{
	// TODO Fehler abfangen
	// TODO in ein Skript auslagern, denn hier sollte hwclock geschrieben
	// werden.

	QString s = "sudo date -s '" + QString::number (dt.date ().year ()) + "-" + QString::number (dt.date ().month ()) + "-"
			+ QString::number (dt.date ().day ()) + " " + QString::number (dt.time ().hour ()) + ":" + QString::number (
			dt.time ().minute ()) + ":" + QString::number (dt.time ().second ()) + "'";

	system (s.latin1());

	// TODO Ja, genau.
	show_warning ("Die Systemzeit wurde ge�ndert. Um die �nderung dauerhaft\n"
		"zu speichern, muss der Rechner einmal heruntergefahren\n"
		"werden, bevor er ausgeschaltet wird.", this);

	//	s="sudo /etc/init.d/hwclock.sh stop &";
	//	r=system (s.c_str ());
	//	if (r!=0) show_warning ("Zur�ckschreiben der Zeit fehlgeschlagen. Bitte den Rechner\n"
	//	                        "herunterfahren, um die Zeiteinstellung dauerhaft zu speichern.", this);

	update_info ();
	update_time ();
}

/**
 * End the program after confirming.
 */
void MainWindow::slot_close ()
{
	if (QMessageBox::information (this, "Wirklich beenden?", "Programm wirklich beenden?", "&Ja", "&Nein") == 0)
	{
		writeSettings ();
		QCoreApplication::quit ();
	}
}

void MainWindow::slot_shutdown ()
/*
 * Shutdown the system after confirming. The shutdown is done by the
 * shutdown () function.
 */
{
	if (QMessageBox::information (this, "Wirklich herunterfahren?", "System wirklich herunterfahren?", "&Ja", "&Nein")
			== 0)
	{
		writeSettings ();
		shutdown ();
	}
}

void MainWindow::shutdown ()
/*
 * Shut down the program. This is done by returning a magic number to the
 * shell.
 */
{
	QApplication::exit (69);
}

// Qt3
//#define toggle(menu, id) do {} while (false)
// Qt4
//#define toggle(menu, id) do { menu->setItemChecked (id, !menu->isItemChecked (id)); } while (false)

/**
 * Toggle whether landed flights are displayed in the Table. Apply the
 * change.
 */
void MainWindow::slot_menu_ansicht_flug_gelandete (bool toggle)
{
	tbl_fluege->gelandete_ausblenden = toggle;
	if (toggle)
		actionSuppressLanded->setText ("Gelandete/Weggeflogene Fl�ge anzeigen");
	else
		actionSuppressLanded->setText ("Gelandete/Weggeflogene Fl�ge ausb&lenden");
	update_checks ();
	slot_refresh_table ();
}

/**
 * Toggle whether flights which are flown away are displayed in the Table.
 * Apply the change.
 */
void MainWindow::slot_menu_ansicht_flug_weggeflogene_gekommene (bool toggle)
{
	tbl_fluege->weggeflogene_gekommene_anzeigen = toggle;
	update_checks ();
	slot_refresh_table ();
}

/**
 * Toggle whether erroneous flights are displayed in the Table regardless of
 * their state. Apply the change.
 */
void MainWindow::slot_menu_ansicht_flug_fehlerhafte (bool toggle)
{
	tbl_fluege->fehlerhafte_immer = toggle;
	update_checks ();
	slot_refresh_table ();
}

void MainWindow::slot_menu_ansicht_datum_einstellen ()
/*
 * Open the date editor to hange the date for which flights are displayed.
 */
{
	// TODO Datumseinstellcode
	// anzeigedatum
	DateWindow editor (this, anzeigedatum);
	QObject::connect (&editor, SIGNAL (date_accepted (QDate)), this, SLOT (set_anzeigedatum (QDate)));
	editor.edit_date ();
}

void MainWindow::set_anzeigedatum (QDate date)
/*
 * Change the date for which flights are displayed. Apply the change.
 * Parameters:
 *   - date: the date to set.
 */
{
	if (date.isNull ())
	{
		log_error ("null date in sk_win_main::set_anzeigedatum ()");
	}
	else
	{
		tbl_fluege->set_anzeigedatum (date);
		if (date != anzeigedatum)
		{
			QDate old_date = anzeigedatum;
			anzeigedatum = date;
			if (anzeigedatum != old_date) slot_refresh_table ();
			// TODO infoframe?
		}
	}
}

void MainWindow::slot_menu_ansicht_display_new_flight_date ()
/*
 * Toggles whether the display date is reset to the current date whenever a
 * new flight is created.
 */
{
	//	toggle (menu_ansicht_datum, id_mnu_ansicht_display_new_flight_date);

	//	display_new_flight_date=!menu_ansicht_datum->isItemChecked (id_mnu_ansicht_display_new_flight_date);
	update_checks ();
}

/**
 * sets the default font of the application
 */
void MainWindow::slotSetFont ()
{
	bool ok;
	QFont font = QApplication::font ();
	font = QFontDialog::getFont (&ok, font, this);
	if (ok)
	{
		// the user clicked OK and font is set to the font the user selected
		QApplication::setFont (font);
		tbl_fluege->setFont (font);
	}
	else
	{
		// the user canceled the dialog; font is set to the initial
		// value, in this case Helvetica [Cronyx], 10
	}
}

void MainWindow::slotIncFont ()
{
	QFont font = QApplication::font ();
	int size = font.pointSize ();
	font.setPointSize (size + 1);
	QApplication::setFont (font);
	tbl_fluege->setFont (font);
}

void MainWindow::slotDecFont ()
{
	QFont font = QApplication::font ();
	int size = font.pointSize ();
	if (size > 5)
	{
		font.setPointSize (size - 1);
		QApplication::setFont (font);
		tbl_fluege->setFont (font);
	}
}

void MainWindow::slot_menu_ansicht_always_use_current_date ()
/*
 * Toggles whether the display date is reset to the current date whenever a
 * new flight is created.
 */
{
	//	toggle (menu_ansicht_datum, id_mnu_ansicht_always_use_current_date);

	//	always_use_current_date=!menu_ansicht_datum->isItemChecked (id_mnu_ansicht_always_use_current_date);
	update_checks ();
}

void MainWindow::slot_schleppref_springen ()
/*
 * If the currently selected entry in the Table is a member of an airtow,
 * jump to the other member.
 */
{
	int row = tbl_fluege->currentRow ();
	db_id id = tbl_fluege->id_from_row (row);
	db_id sref = tbl_fluege->schleppref_from_row (row);
	int target_row;

	if (id_invalid (id))
		target_row = tbl_fluege->row_from_id (sref); // Schlepp
	else
		target_row = tbl_fluege->row_from_sref (id); // Flug

	if (target_row >= 0) tbl_fluege->setCurrentCell (target_row, tbl_fluege->currentColumn ());
}

void MainWindow::slot_tabelle_sortieren ()
/*
 * Sort the Table by the currently selected column.
 */
{
	int col = tbl_fluege->currentColumn ();
	tbl_fluege->sortByColumn (col);

}

// Men� Statistik
void MainWindow::slot_pilot_log ()
/*
 * Opens the pilot log statistics window for the current display date.
 */
{
	(new StatisticsWindow (this, "fbf", false, Qt::WDestructiveClose, ss, db))->pilotLog (anzeigedatum);
}

void MainWindow::slot_plane_log ()
/*
 * Opens the plane log statistics window for the current display date.
 */
{
	(new StatisticsWindow (this, "bbf", false, Qt::WDestructiveClose, ss, db))->planeLog (anzeigedatum);
}

void MainWindow::slot_sastat ()
/*
 * Opens the launch type statistics window for the current display
 * date.
 */
{
	(new StatisticsWindow (this, "sas", false, Qt::WDestructiveClose, ss, db))->sastat (anzeigedatum);
}

// Men� Datenbank
void MainWindow::slot_flugzeugeditor ()
/*
 * Opens the plane editor.
 */
{
	flugzeug_liste->liste ();
}

void MainWindow::slot_personeneditor ()
/*
 * Opens the person editor.
 */
{
	personen_liste->liste ();
}

//void MainWindow::slot_csv ()
//	/*
//	 * Old function to create a CSV Table. Now done by the webinterface.
//	 */
//{
//	int r=system ("tools/make_csv");
//
//	if (r!=0)
//	{
//		show_warning ("Fehler bei der Erstellung der CSV-Ausz�ge.", this);
//	}
//}

void MainWindow::slot_db_refresh_all ()
/*
 * Start a refresh of the complete database.
 * Note that this function does not do any refreshing but emits a signal
 * which is propageted to all windows by the db_change mechanism.
 * It is intended to be called by the UI.
 */
{
	DbEvent event (det_refresh, db_alle, 0);
	emit db_change (&event);
}

// Men� Hilfe
void MainWindow::slot_info ()
/*
 * Show the information dialog with the program version information.
 */
{
	ss->show_version ();
}

void MainWindow::slot_netztest ()
/*
 * Execute the external network testing utility, if set.
 */
{
	if (!opts.diag_cmd.isEmpty ()) system (opts.diag_cmd.latin1());
}

// Men� Debug
void MainWindow::slot_segfault ()
/*
 * Cause a segfault.
 * This function is intended for testing the automatic restart mechanism.
 */
{
	*(int *)NULL = 0;
}

void MainWindow::slot_ping_db ()
/*
 * Check if the database connection is OK and inform the user.
 */
{
	if (db->alife ())
	{
		QMessageBox::information (this, "Datenbankstatus", "Datenbank lebt"); //, "&Ja", "&Nein", QString::null, 0, 1);
	}
	else
	{
		QMessageBox::information (this, "Datenbankstatus", "Datenbank ist tot"); //, "&Ja", "&Nein", QString::null, 0, 1);
	}
}

void MainWindow::slot_display_log ()
{
	//	toggle (menu_debug, id_mnu_debug_display_log);

	//	display_log=!menu_debug->isItemChecked (id_mnu_debug_display_log);
	update_checks ();
	if (display_log)
		log->show ();
	else
		log->hide ();
}


// Men� Demosystem
void MainWindow::slot_webinterface ()
/*
 * Calls a browser with the web interface.
 */
{
	system ("firefox http://localhost/ &");
}

// Logging
void MainWindow::log_message (QString message)
{
	log->append ("[" + QTime::currentTime ().toString () + "] " + message);
}

void MainWindow::log_message (QString *message)
{
	if (message) log_message (*message);
}

// Auxiliary functions
bool MainWindow::try_initialize_db (QString reason)
// Returns: success
{
	// We need to initialize the database. Therefore, we ask the
	// root password from the user.
	QString caption = "root-Passwort ben�tigt";
	QString user_display = opts.root_name + "@" + opts.server_display_name;
	QString label = "Die Datenbank " + opts.database + " ist nicht benutzbar. Grund:\n" + reason + "\n"
			+ "Zur Korrektur wird das Password von " + user_display + " ben�tigt.";

	bool wrong_password_do_it_again;
	do
	{
		wrong_password_do_it_again = false;
		bool ok;
		// TODO when a password was given, don't ask.
		QString root_pass =
				QInputDialog::getText (caption, label, QLineEdit::Password, QString::null, &ok, this);
		if (ok)
		{
			// OK pressed
			try
			{
				Database root_db;
				root_db.display_queries = opts.display_queries;
				root_db.set_database (opts.database);
				root_db.set_connection_data (opts.server, opts.port, opts.root_name, root_pass);

				initialize_database (root_db);
			}
			catch (Database::ex_access_denied &e)
			{
				wrong_password_do_it_again = true;
				label = e.description (true) + "\nPasswort f�r " + opts.root_name + "@"
						+ opts.server_display_name + ":";
			}
			catch (Database::ex_init_failed &e)
			{
				db_error = e.description (true);
				QMessageBox::critical (this, e.description (true), db_error, QMessageBox::Ok,
						QMessageBox::NoButton);
				return false;
			}
			// TODO show output from creation
			catch (SkException &e)
			{
				// Database initialization failed. That means that there is no point in
				// trying the connection again.
				db_error = "Datenbankfehler: " + e.description ();
				QMessageBox::critical (this, "Datenbankfehler", db_error, QMessageBox::Ok,
						QMessageBox::NoButton);
				return false;
			}
		}
		else
		{
			// Cancel pressed
			db_error = "Datenbank nicht benutzbar";
			return false;
		}
	} while (wrong_password_do_it_again);

	return true;
}

bool MainWindow::db_available ()
{
	return (db_state == ds_established);
}

void MainWindow::set_connection_label (const QString &text, const QColor &color)
{
	if (lbl_info_value[idx_info_datenbankverbindung]) lbl_info_value[idx_info_datenbankverbindung]->setText (text);
	if (lbl_info_value[idx_info_datenbankverbindung]) lbl_info_value[idx_info_datenbankverbindung]->setPaletteForegroundColor (
			color);
	// TODO auch dieser Aufruf f�hrt dazu, dass beim Starten manchmal das
	// Fenster verkleinert wird. WTF?
	//	lbl_info_value[idx_info_datenbankverbindung]->adjustSize ();
	// TODO hier wurde beim Starten manchmal das Fenster verkleinert.
	// Vielleicht kann man es auch einfach weglassen?
	//	qApp->processEvents ();
}

////////////////////////////////////////////////////////////////
//////////////// NEW DATABASE STATE CODE ///////////////////////
////////////////////////////////////////////////////////////////

// FSM functions
void MainWindow::slot_timer_db ()
{
	// When the timer is triggered, do the action correspondig to the current
	// state.
	db_do_action ();
}

void MainWindow::db_set_state (db_state_t new_state)
// Sets the state and updates other variables
{
	db_state_t old_state = db_state;
	db_state = new_state;

#define SET_STATE(STATE, TIMER, CONTROLS, COLOR, TEXT)	\
	case STATE:	\
		if (TIMER>=0) timer_db->start (TIMER); else timer_db->stop ();	\
		tbl_fluege->setEnabled (CONTROLS);	\
		set_connection_label (QString (TEXT), COLOR);	\
		menu_enables (false);	\
		if (!error_string.isEmpty ()) log_message (error_string);	\
		break;

	static const QColor col_ok (0, 0, 0);
	static const QColor col_error (255, 0, 0);

	QString error_string;
	if (db_error.isEmpty ())
		error_string = db->get_last_error ();
	else
		error_string = db_error;
	db_error = "";

	switch (new_state)
	{
		//         STATE                TIMER  CONTROLS, COLOR,     TEXT
		SET_STATE (ds_uninitialized, 1000, false, col_error, "Nicht hergestellt")
		SET_STATE (ds_no_connection, 2000, false, col_error, "Keine Verbindung") // Try to reconnect after 2 s
		SET_STATE (ds_established, 1000, true, col_ok, "OK")
		SET_STATE (ds_unusable, -1, false, col_error, "Nicht benutzbar")
		SET_STATE (ds_connection_lost, 1000, false, col_error, "Verbindung verloren") // Try to reconnect after 1 s
	}
#undef SET_STATE

	if (new_state == ds_established)
	{
		switch (old_state)
		{
			case ds_uninitialized:
			case ds_no_connection:
				slot_db_refresh_all ();
				//				tbl_fluege->setFocus ();
				break;
			case ds_established:
				break;
			case ds_unusable:
				log_error ("Invalid state transition ds_unusable->ds_established");
				break;
			case ds_connection_lost:
				slot_refresh_table ();
				//				tbl_fluege->setFocus ();
				break;
				// no default to allow compiler warnings
		}
		if (old_state != ds_established) update_info ();
	}

	if (new_state != old_state && new_state != ds_no_connection) db_do_action ();
}

MainWindow::db_state_t MainWindow::db_action_connect ()
{
	//set_connection_label ("Verbindungsaufbau zu "+db->get_server ()+"...");
	set_connection_label ("Verbindungsaufbau zu " + opts.server_display_name + "...");

	bool initialize_and_try_again;
	do
	{
		initialize_and_try_again = false;
		QString reason;

		try
		{
			//try
			{
				db->connect ();
			} //catch (Database::ex_access_denied &e) { throw Database::ex_unusable (e.description ()); }
			db->use_db ();
			db->check_usability ();
		}
		catch (Database::ex_access_denied &e)
		{
			initialize_and_try_again = true;
			reason = e.description ();
		}
		catch (Database::ex_insufficient_access &e)
		{
			initialize_and_try_again = true;
			reason = e.description ();
		}
		catch (Database::ex_unusable &e)
		{
			// The database was found to be unusable, so we need to initialize
			// it (as root) and try again.
			initialize_and_try_again = true;
			reason = e.description ();
		}
		catch (Database::ex_connection_failed &e)
		{
			return ds_no_connection;
		}
		catch (Database::ex_query_failed &e)
		{
			db_error = e.description ();
		}
		catch (SkException &e)
		{
			// ex_allocation_error ex_database_not_accessible
			// ex_parameter_error ex_not_connected ex_query_failed
			db_error = e.description ();
		}

		if (initialize_and_try_again)
		{
			if (!try_initialize_db (reason))
			{
				// Initialization failed.
				// We will not retry, but rather go to ds_unusable state.
				initialize_and_try_again = false;
				return ds_unusable;
			}
		}
	} while (initialize_and_try_again);

	// If we are here, that means we have sucessfully established the
	// connection.
	restart_all_plugins ();
	return ds_established;
}

MainWindow::db_state_t MainWindow::db_action_check_connection ()
{
	if (db->alife ())
		return ds_established;
	else
		return ds_connection_lost;
}

void MainWindow::db_do_action ()
{
	switch (db_state)
	{
		case ds_uninitialized:
			db_set_state (db_action_connect ());
			break;
		case ds_no_connection:
			db_set_state (db_action_connect ());
			break;
		case ds_established:
			db_set_state (db_action_check_connection ());
			break;
		case ds_unusable:
			break; // no action
		case ds_connection_lost:
			db_set_state (db_action_connect ());
			break; // try to reconnect
			// no default to allow compiler warnings
	}
}

void MainWindow::start_db ()
{
	db_do_action ();
}

//void MainWindow::slot_plugin_clicked (int i)
//{
//	plugins[i].restart ();
//}

/**
 * versuch, die virtuelle tastatur zu aktivieren/deaktivieren
 */
void MainWindow::slotToggleKeyboard (bool active)
{
	if (active)
	{
		int result = system ("/usr/bin/dcop kvkbd kvkbd show");
		if (result)
		{
			// failed to show; try launch
			system ("/usr/bin/kvkbd &");
			system ("/usr/bin/dcop kvkbd kvkbd show");
		}
	}
	else
		system ("/usr/bin/dcop kvkbd kvkbd hide");
}

// *************
// ** Plugins **
// *************

void MainWindow::restart_all_plugins ()
{
	if (weather)
	{
		weather_plugin->restart ();
	}

	QMutableListIterator<ShellPlugin> plugin (*plugins);
	while (plugin.hasNext ())
		plugin.next().restart();
}

void MainWindow::openWeatherDialog ()
{
	if (!opts.weather_dialog_plugin.isEmpty ())
	{
		ShellPlugin *weather_ani_plugin = new ShellPlugin (opts.weather_dialog_title, opts.weather_dialog_plugin,
				opts.weather_dialog_interval); // Initialize to given values

		if (weatherDialog) delete weatherDialog;
		weatherDialog = new WeatherDialog (weather_ani_plugin, this, "weatherDialog");
		weatherDialog->setCaption (opts.weather_dialog_title);
		weatherDialog->show ();
	}
}

/**
 * read config settings
 */
void MainWindow::readSettings ()
{
	QSettings settings ("startkladde", "startkladde");
	settings.beginGroup ("fonts");
	QString fontDescription = settings.value ("font").toString ();
	QFont font;
	if (font.fromString (fontDescription))
	{
		QApplication::setFont (font);
		tbl_fluege->setFont (font);
	}
	settings.endGroup ();
	tbl_fluege->readSettings (settings);
}

/**
 * write config settings
 */
void MainWindow::writeSettings ()
{
	QSettings settings ("startkladde", "startkladde");
	settings.beginGroup ("fonts");
	QFont font = QApplication::font ();
	settings.setValue ("font", font.toString ());
	settings.endGroup ();
	tbl_fluege->writeSettings (settings);
	settings.sync ();
}
