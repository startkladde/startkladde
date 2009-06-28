#include "FlightWindow.h"

#include <QLayout>
#include <QApplication>

#define printSizeHint(w) do { QSize size=w->sizeHint (); cout << #w " size hint: " << size.width () << "x" << size.height () << endl; } while (0)

const QColor col_obligatory (224, 224, 224);
const QColor col_erroneous (255, 127, 127);

// Symbolic constants for widget labels/buttons
const char *text_registration="&Kennzeichen";
const char *text_flugtyp="Flugt&yp";
const char *text_nachname_1="&Nachname";
const char *text_vorname_1="&Vorname";
const char *text_nachname_2="Nac&hname";
const char *text_vorname_2="Vorna&me";
const char *text_nachname_3="Nachname";
const char *text_vorname_3="Vorname";
const char *text_modus="Mo&dus";
const char *text_startart="Starta&rt";
const char *text_registration_sfz="Kennzeichen Sch&leppflugzeug";
const char *text_typ_sfz="Typ";
const char *text_schleppflug_modus="Schlepp&flug Modus";
const char *text_startzeit="Startze&it";
const char *text_landezeit="Lande&zeit";
const char *text_schleppende="Schl&eppende";
const char *text_landezeit_schleppflugzeug="Landezeit Schl&eppflugzeug";
const char *text_startort="Sta&rtort";
const char *text_zielort="Zie&lort";
const char *text_zielort_schleppflugzeug="&Zielort Schleppflugzeug";
const char *text_landungen="Landun&gen";
const char *text_bemerkungen="&Bemerkungen";
const char *text_abrechnungshinweis="Abrechnungshin&weis";
const char *text_datum="Dat&um";
const char *text_fehler="F&ehler";
const char *text_automatisch_1="Automati&sch";
const char *text_gestartet="Ge&startet";
const char *text_automatisch_2="&Automatisch";
const char *text_gelandet_1="Gel&andet";
const char *text_automatisch_3="Au&tomatisch";
const char *text_gelandet_2="Gelande&t";
const char *text_beendet="Beende&t";
const char *text_spaeter_starten="S&p�ter starten";
const char *text_spaeter_landen="S&p�ter landen";
const char *text_jetzt_starten="&Jetzt starten";
const char *text_jetzt_landen="&Jetzt landen";
const char *text_spaeter_="S&p�ter ";
const char *text_jetzt_="&Jetzt ";
const char *text_landen="landen";
const char *text_starten="starten";
const char *text_ok="&OK";
const char *text_abbrechen="Abbre&chen";
const char *text_schliessen="S&chlie�en";

const QString unknown_startart_text="(Unbekannt)";
const QString registration_prefix="D-";

FlightWindow::FlightWindow (QWidget *parent, Database *_db, const char *name, bool modal, WFlags f, QObject *status_dlg)
	:SkDialog (parent, name, modal, f, status_dlg)
	/*
	 * Creates the flight editor window.
	 * Parameters:
	 *   - parent: the parent widget of the editor window. Passed to the base
	 *     class constructor.
	 *   - name: the widget name. Passed to the base class constructor.
	 *   - modal: if the window is modal. Passed to the base class constructor.
	 *   - f: window flags. Passed to the base class constructor.
	 *   - status_dlg: the status window used for displaying status/progress
	 *     information.
	 */
{
	flight=NULL;

	mode=fe_none;
	flight=NULL;
	selected_plane=NULL;
	selected_towplane=NULL;
	original_pilot_id=original_begleiter_id=original_towpilot_id=invalid_id;
	anzahl_pilot=anzahl_begleiter=anzahl_towpilot=0;
	unknown_startart_index=-1;
	lock_edit_slots=false;

	db=_db;
	disable_error_check=false;

	status_dialog=status_dlg;

	hide ();

	// Scrollbar anlegen
	backgroundLayout=new QVBoxLayout (this);
	backgroundLayout->setMargin (0);

	// Editorwidget anlegen
	QWidget *editorWidget=new QWidget ();

	// ScrollArea anlegen
	scrollArea=new QScrollArea (this);
	scrollArea->setWidget (editorWidget);
	scrollArea->setWidgetResizable (true);
	backgroundLayout->addWidget (scrollArea);


	// Editor-Widgets anlegen
	// TODO Codeduplikation Reihenfolge
	int i=0;
	edit_widget[i++]=edit_registration=new SkComboBox (true, editorWidget);
	edit_widget[i++]=edit_flugzeug_typ=new QLabel (editorWidget);
	edit_widget[i++]=edit_flug_typ=new SkComboBox (false, editorWidget);
	edit_widget[i++]=edit_pilot_nn=new lbl_cbox (text_nachname_1, true, editorWidget, "edit_pilot_nn");
	edit_widget[i++]=edit_pilot_vn=new lbl_cbox (text_vorname_1, true, editorWidget, "edit_pilot_vn");
	edit_widget[i++]=edit_begleiter_nn=new lbl_cbox (text_nachname_2, true, editorWidget, "edit_begleiter_nn");
	edit_widget[i++]=edit_begleiter_vn=new lbl_cbox (text_vorname_2, true, editorWidget, "edit_begleiter_vn");
	edit_widget[i++]=edit_modus=new SkComboBox (false, editorWidget);
	edit_widget[i++]=edit_startart=new SkComboBox (false, editorWidget);
	edit_widget[i++]=edit_registration_sfz=new SkComboBox (true, editorWidget);
	edit_widget[i++]=edit_typ_sfz=new QLabel (editorWidget);
	// Towpilot is always added, but not displayed when !opts.record_towpilot
	edit_widget[i++]=edit_towpilot_nn=new lbl_cbox (text_nachname_3, true, editorWidget, "edit_towpilot_nn");
	edit_widget[i++]=edit_towpilot_vn=new lbl_cbox (text_vorname_3, true, editorWidget, "edit_towpilot_vn");
	edit_widget[i++]=edit_modus_sfz=new SkComboBox (false, editorWidget);
	edit_widget[i++]=edit_startzeit=new SkTimeEdit (editorWidget, "startzeit");
	edit_widget[i++]=edit_landezeit=new SkTimeEdit (editorWidget, "landezeit");
	edit_widget[i++]=edit_landezeit_sfz=new SkTimeEdit (editorWidget, "sfz_lande");
	edit_widget[i++]=edit_startort=new SkComboBox (true, editorWidget);
	edit_widget[i++]=edit_zielort=new SkComboBox (true, editorWidget);
	edit_widget[i++]=edit_zielort_sfz=new SkComboBox (true, editorWidget);
	edit_widget[i++]=edit_landungen=new QLineEdit (editorWidget);
	edit_widget[i++]=edit_bemerkungen=new SkTextBox (editorWidget);
	edit_widget[i++]=edit_abrechnungshinweis=new SkComboBox (true, editorWidget);
	edit_widget[i++]=edit_datum=new QDateEdit (editorWidget);
	edit_widget[i++]=edit_fehler=new SkListWidget (editorWidget);


	// Editor-Widgets einstellen
	edit_registration->setAutoCompletion (true);
	edit_pilot_nn->setAutoCompletion (true);
	edit_pilot_vn->setAutoCompletion (true);
	edit_begleiter_nn->setAutoCompletion (true);
	edit_begleiter_vn->setAutoCompletion (true);
	edit_towpilot_nn->setAutoCompletion (true);
	edit_towpilot_vn->setAutoCompletion (true);
	edit_startort->setAutoCompletion (true);
	edit_zielort->setAutoCompletion (true);
	edit_registration_sfz->setAutoCompletion (true);
	edit_zielort_sfz->setAutoCompletion (true);
	edit_abrechnungshinweis->setAutoCompletion (true);
	edit_landungen->setValidator (new QIntValidator (0,9999,edit_landungen));

	edit_registration->setInsertionPolicy (QComboBox::NoInsertion);
	edit_startort->setInsertionPolicy (QComboBox::NoInsertion);
	edit_zielort->setInsertionPolicy (QComboBox::NoInsertion);
	edit_registration_sfz->setInsertionPolicy (QComboBox::NoInsertion);
	edit_zielort_sfz->setInsertionPolicy (QComboBox::NoInsertion);
	edit_abrechnungshinweis->setInsertionPolicy (QComboBox::NoInsertion);

	edit_startzeit->set_pbut_text ("Jetzt starten");
	edit_landezeit->set_pbut_text ("Jetzt landen");
	edit_landezeit_sfz->set_pbut_text ("Jetzt landen");

	//edit_datum->setDisplayFormat ("yyyy-MM-dd");

	edit_fehler->setSelectionMode (QListWidget::NoSelection);	// Single
//	edit_fehler->setFixedHeight (edit_fehler->minimumSize ().height ());
	edit_fehler->setFixedHeight (80); // MURX: besser "4*Zeilenh�he
	//edit_fehler->setAutoScrollBar (true);
	//printSizeHint (edit_fehler);

	// Set the width of the secondary labels
	qApp->processEvents ();	// Need to give the layout manager time to set the label widths.
	int max_name_label_width=0;
	int name_label_with=0;
	name_label_with=edit_pilot_nn     ->get_label_width (); if (name_label_with>max_name_label_width) max_name_label_width=name_label_with;
	name_label_with=edit_pilot_vn     ->get_label_width (); if (name_label_with>max_name_label_width) max_name_label_width=name_label_with;
	name_label_with=edit_begleiter_nn ->get_label_width (); if (name_label_with>max_name_label_width) max_name_label_width=name_label_with;
	name_label_with=edit_begleiter_vn ->get_label_width (); if (name_label_with>max_name_label_width) max_name_label_width=name_label_with;
	name_label_with=edit_towpilot_nn  ->get_label_width (); if (name_label_with>max_name_label_width) max_name_label_width=name_label_with;
	name_label_with=edit_towpilot_vn  ->get_label_width (); if (name_label_with>max_name_label_width) max_name_label_width=name_label_with;
	max_name_label_width+=4;
	edit_pilot_nn     ->set_label_width (max_name_label_width);
	edit_pilot_vn     ->set_label_width (max_name_label_width);
	edit_begleiter_nn ->set_label_width (max_name_label_width);
	edit_begleiter_vn ->set_label_width (max_name_label_width);
	edit_towpilot_nn  ->set_label_width (max_name_label_width);
	edit_towpilot_vn  ->set_label_width (max_name_label_width);


	// Editor-Labels anlegen
	for (int i=0; i<NUM_FIELDS; i++)
	{
		label[i]=new SkLabel (get_default_color (edit_widget[i]), col_erroneous, editorWidget);
		label[i]->setBuddy (edit_widget[i]);
	}

	// Editor-Labels einstellen
	// TODO Codeduplikation Reihenfolge
	i=0;
	label[i++]->setText (text_registration);
	label[i++]->setText ("Flugzeugtyp");	// TODO constants
	label[i++]->setText (text_flugtyp);
	label[i++]->setText ("Pilot");
	label[i++]->setText ("");
	label[i++]->setText ("Begleiter");
	label[i++]->setText ("");
	label[i++]->setText (text_modus);
	label[i++]->setText (text_startart);
	label[i++]->setText (text_registration_sfz);
	label[i++]->setText (text_typ_sfz);
	label[i++]->setText ("Schleppilot");
	label[i++]->setText ("");
	label[i++]->setText (text_schleppflug_modus);
	label[i++]->setText (text_startzeit);
	label[i++]->setText (text_landezeit);
	label[i++]->setText (text_landezeit_schleppflugzeug);
	label[i++]->setText (text_startort);
	label[i++]->setText (text_zielort);
	label[i++]->setText (text_zielort_schleppflugzeug);
	label[i++]->setText (text_landungen);
	label[i++]->setText (text_bemerkungen);
	label[i++]->setText (text_abrechnungshinweis);
	label[i++]->setText (text_datum);
	label[i++]->setText (text_fehler);


	// Layout
	QGridLayout *layout_edit=new QGridLayout (editorWidget, 0, 2, widget_border, widget_gap, "layout_edit");
	for (int i=0; i<NUM_FIELDS; i++)
	{
		layout_edit->addWidget (label[i], i, 0);
		layout_edit->addWidget (edit_widget[i], i, 1);
	}
	layout_edit->setRowStretch (NUM_FIELDS, 1);

	for (int i=0; i<NUM_FIELDS; ++i)
	{
		// Set the height of the row to the height of the label/widget
		// (whichever is higher) so the widget can be hidden without disturbing
		// the layout.
		int label_height=label[i]->sizeHint ().height ();
		int widget_height=edit_widget[i]->sizeHint ().height ();
		layout_edit->setRowSpacing (i, std::max (label_height, widget_height));
	}


		// TODO Unregelm��iger Abstand
//		if (
//				edit_widget[i]==edit_pilot_nn||
//				edit_widget[i]==edit_bemerkungen||
//				edit_widget[i]==edit_modus||
//				edit_widget[i]==edit_fehler||
//				edit_widget[i]==edit_startort||
//				false) top+=large_gap;
//		else if (
//				edit_widget[i]==edit_startart||
//				edit_widget[i]==edit_modus_sfz||
//				edit_widget[i]==edit_begleiter_nn||
//				edit_widget[i]==edit_landungen||
//				edit_widget[i]==edit_datum||
//				false) top+=small_gap;
//		else top+=widget_gap;

//	}

	// Buttons anlegen und positionieren
	QHBoxLayout *layout_buttons=new QHBoxLayout ();
	layout_buttons->setMargin (9);

	but_ok=new QPushButton (text_ok, this);
	but_cancel=new QPushButton (text_abbrechen, this);
	but_later=new QPushButton (text_spaeter_starten, this);

	layout_buttons->addStretch (1);
	layout_buttons->add (but_later);
	layout_buttons->add (but_ok);
	layout_buttons->add (but_cancel);

	backgroundLayout->addLayout (layout_buttons);
//	layout_edit->addMultiCellLayout (layout_buttons, NUM_FIELDS+1, NUM_FIELDS+1, 0, 1);

	but_ok->setDefault (true);



	// Dialog auf beste Gr��e setzen
	//printSizeHint (editorWidget);
	//printSizeHint (this);
	//printSizeHint (backgroundLayout);
	//printSizeHint (scrollArea);

	// H�he einstellen
	QSize size=backgroundLayout->sizeHint ();
	int height=size.height ();

	height+=editorWidget->sizeHint ().height ()-scrollArea->sizeHint ().height (); // Falsche Gr��e der ScrollArea ausgleichen
	height+=20;	// Pfusch, aber sonst ists ein paar Pixel zu klein

	int desktopHeight=(QApplication::desktop ())->availableGeometry ().height ();
//	desktopHeight-=20; // Pfusch
//	cout << "desktop height: " << desktopHeight << endl;
	if (height>desktopHeight) height=desktopHeight;

	size.setHeight (height);
	resize (size);




	populate_lists ();
	reset ();

	// Signale verbinden
	QObject::connect (but_cancel, SIGNAL(clicked()), this, SLOT(slot_cancel ()));
	QObject::connect (but_ok, SIGNAL(clicked()), this, SLOT(slot_ok ()));
	QObject::connect (but_later, SIGNAL(clicked()), this, SLOT(slot_later ()));

	QObject::connect (edit_registration, SIGNAL (focus_out ()), this, SLOT (slot_registration ()));
	QObject::connect (edit_registration, SIGNAL (focus_in ()), this, SLOT (slot_registration_in ()));
	QObject::connect (edit_registration, SIGNAL (activated (const QString &)), this, SLOT (slot_registration ()));
	QObject::connect (edit_flug_typ, SIGNAL (activated (int)), this, SLOT (slot_flugtyp (int)));
	QObject::connect (edit_modus, SIGNAL (activated (int)), this, SLOT (slot_modus (int)));
	QObject::connect (edit_startart, SIGNAL (activated (int)), this, SLOT (slot_startart (int)));
	QObject::connect (edit_registration_sfz, SIGNAL (focus_out ()), this, SLOT (slot_registration_sfz ()));
	QObject::connect (edit_registration_sfz, SIGNAL (focus_in ()), this, SLOT (slot_registration_sfz_in ()));
	QObject::connect (edit_registration_sfz, SIGNAL (activated (const QString &)), this, SLOT (slot_registration_sfz ()));
	QObject::connect (edit_modus_sfz, SIGNAL (activated (int)), this, SLOT (slot_modus_sfz (int)));
	QObject::connect (edit_startzeit, SIGNAL (cbox_clicked ()), this, SLOT (slot_gestartet ()));
	QObject::connect (edit_startzeit, SIGNAL (time_changed ()), this, SLOT (slot_startzeit ()));
	QObject::connect (edit_landezeit, SIGNAL (cbox_clicked ()), this, SLOT (slot_gelandet ()));
	QObject::connect (edit_landezeit, SIGNAL (time_changed ()), this, SLOT (slot_landezeit ()));
	QObject::connect (edit_landezeit_sfz, SIGNAL (cbox_clicked ()), this, SLOT (slot_sfz_gelandet ()));
	QObject::connect (edit_landezeit_sfz, SIGNAL (time_changed ()), this, SLOT (slot_landezeit_sfz ()));
	QObject::connect (edit_startort, SIGNAL (focus_out ()), this, SLOT (slot_startort ()));
	QObject::connect (edit_zielort, SIGNAL (focus_out ()), this, SLOT (slot_zielort ()));
	QObject::connect (edit_zielort_sfz, SIGNAL (focus_out ()), this, SLOT (slot_zielort_sfz ()));
	QObject::connect (edit_landungen, SIGNAL (lostFocus ()), this, SLOT (slot_landungen ()));
	QObject::connect (edit_datum, SIGNAL (dateChanged (const QDate &)), this, SLOT (slot_datum ()));
	QObject::connect (edit_bemerkungen, SIGNAL (focus_out ()), this, SLOT (slot_bemerkung ()));
	QObject::connect (edit_abrechnungshinweis, SIGNAL (focus_out ()), this, SLOT (slot_abrechnungshinweis ()));

	// Oh je, das ist ja f�rchterlich (3*2*3). W�re da nicht ein wenig Zusammenfassen angebracht?
	QObject::connect (edit_pilot_vn, SIGNAL (focus_out ()), this, SLOT (slot_pilot_vn ()));
//	QObject::connect (edit_pilot_vn, SIGNAL (activated (const QString &)), this, SLOT (slot_pilot_vn ()));
	QObject::connect (edit_pilot_vn, SIGNAL (progress (int, int)), this, SIGNAL (progress (int, int)));
	QObject::connect (edit_pilot_nn, SIGNAL (focus_out ()), this, SLOT (slot_pilot_nn ()));
//	QObject::connect (edit_pilot_nn, SIGNAL (activated (const QString &)), this, SLOT (slot_pilot_nn ()));
	QObject::connect (edit_pilot_nn, SIGNAL (progress (int, int)), this, SIGNAL (progress (int, int)));
	QObject::connect (edit_begleiter_vn, SIGNAL (focus_out ()), this, SLOT (slot_begleiter_vn ()));
//	QObject::connect (edit_begleiter_vn, SIGNAL (activated (const QString &)), this, SLOT (slot_begleiter_vn ()));
	QObject::connect (edit_begleiter_vn, SIGNAL (progress (int, int)), this, SIGNAL (progress (int, int)));
	QObject::connect (edit_begleiter_nn, SIGNAL (focus_out ()), this, SLOT (slot_begleiter_nn ()));
//	QObject::connect (edit_begleiter_nn, SIGNAL (activated (const QString &)), this, SLOT (slot_begleiter_nn ()));
	QObject::connect (edit_begleiter_nn, SIGNAL (progress (int, int)), this, SIGNAL (progress (int, int)));
	QObject::connect (edit_towpilot_vn, SIGNAL (focus_out ()), this, SLOT (slot_towpilot_vn ()));
//	QObject::connect (edit_towpilot_vn, SIGNAL (activated (const QString &)), this, SLOT (slot_towpilot_vn ()));
	QObject::connect (edit_towpilot_vn, SIGNAL (progress (int, int)), this, SIGNAL (progress (int, int)));
	QObject::connect (edit_towpilot_nn, SIGNAL (focus_out ()), this, SLOT (slot_towpilot_nn ()));
//	QObject::connect (edit_towpilot_nn, SIGNAL (activated (const QString &)), this, SLOT (slot_towpilot_nn ()));
	QObject::connect (edit_towpilot_nn, SIGNAL (progress (int, int)), this, SIGNAL (progress (int, int)));


	QObject::connect (edit_startzeit, SIGNAL (clicked ()), edit_startzeit, SLOT (set_current_time ()));
	QObject::connect (edit_landezeit, SIGNAL (clicked ()), edit_landezeit, SLOT (set_current_time ()));
	QObject::connect (edit_landezeit_sfz, SIGNAL (clicked ()), edit_landezeit_sfz, SLOT (set_current_time ()));

	QObject::connect (edit_startzeit, SIGNAL (cbox_clicked ()), this, SLOT (setup_controls ()));
	QObject::connect (edit_landezeit, SIGNAL (cbox_clicked ()), this, SLOT (setup_controls ()));


	edit_widget[0]->setFocus ();
}

FlightWindow::~FlightWindow ()
{
	// Widgets are deleted by the QT library.
}

void FlightWindow::reset ()
	/*
	 * Reset all controls to their default value.
	 * Actually, this should be done in the constructor. However, this window is
	 * not recreated every time it is used because it has to query the database
	 * which takes too much time. Thus, the window is kept open and reset before
	 * displaying using this function.
	 */
{
	for (int i=0; i<NUM_FIELDS; i++) label[i]->set_error (false);

	// Lock the edit slots so they don't get executed, entering random values
	// into the flight.
	bool old_lock_edit_slots=lock_edit_slots;
	lock_edit_slots=true;

	edit_registration->setCurrentText (registration_prefix);
	edit_registration->cursor_to_end ();
	edit_flugzeug_typ->setText ("");
	edit_flug_typ->setCurrentItem (flugtyp_index (ftNormal));
	edit_pilot_vn->use_full (true);
	edit_pilot_vn->setCurrentText ("");
	edit_pilot_nn->use_full (true);
	edit_pilot_nn->setCurrentText ("");
	edit_begleiter_vn->use_full (true);
	edit_begleiter_vn->setCurrentText ("");
	edit_begleiter_nn->use_full (true);
	edit_begleiter_nn->setCurrentText ("");

	edit_towpilot_vn->use_full (true);
	edit_towpilot_vn->setCurrentText ("");
	edit_towpilot_nn->use_full (true);
	edit_towpilot_nn->setCurrentText ("");

	edit_startart->setCurrentItem (startart_index (invalid_id));
	edit_registration_sfz->setCurrentText (registration_prefix);
	edit_registration_sfz->cursor_to_end ();
	edit_typ_sfz->setText ("");
	edit_modus->setCurrentItem (modus_index (fmLocal));
	edit_modus_sfz->setCurrentItem (sfz_modus_index (fmLocal));
	edit_startzeit->reset ();
	edit_landezeit->reset ();
	edit_landezeit_sfz->reset ();
	edit_startort->setCurrentText (opts.ort);
	edit_zielort->setEditText ("");
	edit_zielort_sfz->setEditText ("");
	edit_landungen->setText ("");
	edit_bemerkungen->setText ("");
	edit_abrechnungshinweis->setEditText ("");
	// TODO nach UTC einstellen
	edit_datum->setDate (QDate::currentDate ());

	original_pilot_id=original_begleiter_id=original_towpilot_id=invalid_id;

	lock_edit_slots=old_lock_edit_slots;

	// Manually call those slots which affect visibility of other controls
	// TODO this is messy
	slot_flugtyp (edit_flug_typ->currentItem ());
	slot_modus (edit_modus->currentItem ());
	slot_startart (edit_startart->currentItem ());
	slot_modus_sfz (edit_modus_sfz->currentItem ());

	scrollArea->ensureVisible (0, 0, 0, 0);
}

void FlightWindow::set_buttons (bool spaeter_enabled, QString aktion_text, bool read_only)
	/*
	 * Setup the buttons (text and visibility).
	 * Parameters:
	 *   - spaeter_enabled: wheter the starting/landing time is set to "later".
	 *   - aktion_text: the action that is taken when the button is pressed, for
	 *     example "starten". Used for button text.
	 *   - read_only: whether the flight is read-only.
	 */
{
	if (read_only)
	{
		but_later->hide ();
		but_ok->hide ();
		but_cancel->setText (text_schliessen);
	}
	else if (spaeter_enabled)
	{
		but_later->setText (text_spaeter_+aktion_text);
		but_later->show ();
		but_ok->setText (text_jetzt_+aktion_text);
		but_ok->show ();
		but_cancel->setText (text_abbrechen);
		but_later->setDefault (true);
	}
	else
	{
		// Sp�ter (aktion) nicht aktiv, also nur OK
		but_later->hide ();
		but_ok->setText (text_ok);
		but_ok->show ();
		but_cancel->setText (text_abbrechen);
		but_ok->setDefault (true);
	}
}

void FlightWindow::setup_controls (bool init, bool read_only, bool repeat)
	/*
	 * Sets up the controls and member variables, according to the member 'mode'
	 * etc.
	 * Parameters:
	 *   - init: whether this is the first call after opening the window
	 *     with/for a new flight, as opposed to if a widget is changed which
	 *     effects other widgets' settings.
	 *   - read_only: whether the flight is read only.
	 *   - repeat: whether we are repeating a flight, as opposed to
	 *     editing
	 * Members used:
	 *   - mode
	 */
{
	switch (mode)
	{
		case fe_none:
		{
			// This may happen on initialization
		} break;
		case fe_create:
		{
			// TODO: 3*3. Preprocessor?
			edit_startzeit->invert=false;
			edit_landezeit->invert=false;
			edit_landezeit_sfz->invert=false;
			edit_startzeit->set_cbox_text (text_automatisch_1);
			edit_landezeit->set_cbox_text (text_automatisch_2);
			edit_landezeit_sfz->set_cbox_text (text_automatisch_3);
			edit_startzeit->set_mode (tm_box_time);
			edit_landezeit->set_mode (tm_box_time);
			edit_landezeit_sfz->set_mode (tm_box_time);

			enable_widget (edit_fehler, false);

			but_later->setDefault (true);

			setCaption ("Flug anlegen");
		} break;
		case fe_edit:
		{
//			edit_startzeit->set_mode (tm_time_but);
//			edit_landezeit->set_mode (tm_time_but);
//			edit_landezeit_sfz->set_mode (tm_time_but);
			edit_startzeit->invert=true;
			edit_landezeit->invert=true;
			edit_landezeit_sfz->invert=true;
			// TODO Hotkeys
			edit_startzeit->set_cbox_text (text_gestartet);
			edit_landezeit->set_cbox_text (text_gelandet_1);
			// TODO code duplication text

			if (lands_here (sfz_modi[edit_modus_sfz->currentItem ()]))
				edit_landezeit_sfz->set_cbox_text (text_gelandet_2);
			else
				edit_landezeit_sfz->set_cbox_text (text_beendet);

			edit_startzeit->set_mode (tm_box_time);
			edit_landezeit->set_mode (tm_box_time);
			edit_landezeit_sfz->set_mode (tm_box_time);

			enable_widget (edit_fehler, true);

			but_ok->setDefault (true);

			setCaption ("Flug editieren");
		} break;
		default:
		{
			log_error ("Unbehandelter Editiermodus in sk_win_flight::setup_controls ()");
		} break;
	}

	for (int i=0; i<NUM_FIELDS; i++)
		edit_widget[i]->setEnabled (!read_only);
	edit_fehler->setEnabled (true);

	FlightMode fmod=modi[edit_modus->currentItem ()];
	bool sz_auto=edit_startzeit->checked ();
	bool lz_auto=edit_landezeit->checked ();

	if (mode==fe_edit)
	{
		// Beim Flug editieren gibt es nur OK
		set_buttons (false, "", read_only);
	}
	else if (mode==fe_create)
	{
		// Beim Flug erstellen ist das schon etwas komplizierter: da muss man
		// auf den Flugmodus und das Datum schauen.

		if (edit_datum->date ()==QDate::currentDate ())
		{
			switch (fmod)
			{
				case fmLocal:
					set_buttons (sz_auto, text_starten);
					break;
				case fmComing:
					set_buttons (lz_auto, text_landen);
					break;
				case fmLeaving:
					set_buttons (sz_auto, text_starten);
					break;
				default:
					set_buttons (false);
					log_error ("Unbekannter flugmodus in sk_win_flight::setup_controls");
					break;
			}
		}
		else
		{
			// Nachtragen
			set_buttons (false);
		}
	}

	if (init)
	{
		edit_fehler->clear ();

		if (selected_plane) delete selected_plane;
		if (flight)
		{
			selected_plane=new Plane;
			int ret=db->get_plane (selected_plane, flight->flugzeug);
			if (ret<0) { delete selected_plane; selected_plane=NULL; }
		}

		if (selected_towplane) delete selected_towplane;
		if (flight)
		{
			selected_towplane=new Plane;
			int ret=db->get_plane (selected_towplane, flight->towplane);
			if (ret<0) { delete selected_towplane; selected_towplane=NULL; }
		}
	}

	if (init)
	{
		if (mode==fe_create && repeat && starts_here (flight->modus))
			edit_startart->setFocus ();
		else
			edit_widget[0]->setFocus ();
	}
}

void FlightWindow::populate_lists ()
	/*
	 * Fill the list fields which do not need to query the database.
	 */
{
	// Startarten eintragen
	// TODO this does query the database, move to different location?
	unknown_startart_index=-1;	// None

	QPtrList<LaunchType> saen; saen.setAutoDelete (true);
	db->list_startarten_all (saen);

	edit_startart->clear ();
	startarten.clear ();

	if (saen.count ()>1)
	{
		startarten.append (invalid_id);
		edit_startart->insertItem ("---", edit_startart->count ());
	}

	for (QPtrListIterator<LaunchType> sa (saen); *sa; ++sa)
	{
		// TODO das sorgt zwar daf�r, dass die Indizies in startarten und
		// edit_startart �bereinstimmen, aber gut ist das nicht.
		startarten.append ((*sa)->get_id ());
		edit_startart->insertItem ((*sa)->list_text (), edit_startart->count ());
	}


	// Flugmodi eintragen
	// TODO fillstringarraydelete, clear () wegmachen
	edit_modus->clear ();
	modi=listFlightModes (false);
	for (int i=0; i<modi.size (); i++)
		edit_modus->insertItem (flightModeText (modi[i], lsWithShortcut), i);


	// Flugmodi Schleppflugzeug eintragen
	// TODO fillstringarraydelete, clear () wegmachen
	edit_modus_sfz->clear ();
	sfz_modi=listTowFlightModes (false);
	for (int i=0; i<sfz_modi.size(); i++)
		edit_modus_sfz->insertItem (flightModeText (sfz_modi[i], lsWithShortcut), i);


	// Flugtypen eintragen
	// TODO fillstringarraydelete, clear () wegmachen
	edit_flug_typ->clear ();
	flightTypes=listFlightTypes (false);

	for (int i=0; i<num_flugtypen; i++)
		edit_flug_typ->insertItem (flightTypeText (flightTypes[i], lsWithShortcut), i);

}

void FlightWindow::read_db ()
	/*
	 * Read database entries (planes, airfields, persons etc.). Slow for large
	 * amounts of data.
	 */
{
	emit long_operation_start ();

	emit status ("Flugeditor: Personen aus Datenbank lesen...");
	// Personen eintragen
	namen_aus_datenbank (edit_pilot_vn, edit_pilot_nn, edit_begleiter_vn, edit_begleiter_nn, edit_towpilot_vn, edit_towpilot_nn);


	emit status ("Flugeditor: Orte aus Datenbank lesen...");
	// Orte eintragen
	// TODO fillstringarraydelete, clear () wegmachen
	edit_startort->clear ();
	edit_zielort->clear ();
	edit_zielort_sfz->clear ();
	edit_startort->insertItem ("");
	edit_zielort->insertItem ("");
	edit_zielort_sfz->insertItem ("");
	QStringList airfields;
	// TODO error handling
	db->list_airfields (airfields);
	edit_startort->insertStringList (airfields);
	edit_zielort->insertStringList (airfields);
	edit_zielort_sfz->insertStringList (airfields);

	edit_startort->setCurrentText (opts.ort);
	edit_zielort->setCurrentText (opts.ort);		// Damit opts.ort in der Liste steht
	edit_zielort->setCurrentText ("");
	edit_zielort_sfz->setCurrentText ("");

	emit status ("Flugeditor: Abrechnungshinweise aus Datenbank lesen...");
	// Abrechnungshinweise eintragen
	// TODO fillstringarraydelete, clear () wegmachen
	edit_abrechnungshinweis->clear ();
	edit_abrechnungshinweis->insertItem ("");
	QStringList abhins;
	db->list_accounting_note (abhins);
	edit_abrechnungshinweis->insertStringList (abhins);

	edit_abrechnungshinweis->setCurrentText ("");

	emit status ("Flugeditor: Flugzeuge aus Datenbank lesen...");
	// Kennzeichen eintragen
	// TODO fillstringarraydelete, clear () wegmachen
	edit_registration->clear ();
	edit_registration_sfz->clear ();
//	QPtrList<Plane> planes; planes.setAutoDelete (true);
	// TODO error handling
	QStringList registrations;
	db->list_registrations (registrations);

	edit_registration->insertStringList (registrations);
	edit_registration_sfz->insertStringList (registrations);
	edit_registration->setCurrentText ("");
	edit_registration_sfz->setCurrentText ("");

	emit long_operation_end ();
}

void FlightWindow::namen_aus_datenbank (lbl_cbox *vorname, lbl_cbox *nachname, lbl_cbox *vorname2, lbl_cbox *nachname2, lbl_cbox *vorname3, lbl_cbox *nachname3)
	/*
	 * Read all names from the database and write them to the complete lists.
	 * This is slow and should only be done once on initialization.
	 */
{
	if (vorname || vorname2 || vorname3)
	{
		QStringList names;
		// TODO error handling
		db->list_first_names (names);
		if (vorname3) vorname3->fillFullStringArray (names, true, false, false);
		if (vorname2) vorname2->fillFullStringArray (names, true, false, false);
		if (vorname) vorname->fillFullStringArray (names, true, false, true);
	}

	if (nachname || nachname2 || nachname3)
	{
		QStringList names;
		// TODO error handling
		db->list_last_names (names);
		if (nachname3) nachname3->fillFullStringArray (names, true, false, false);
		if (nachname2) nachname2->fillFullStringArray (names, true, false, false);
		if (vorname) nachname->fillFullStringArray (names, true, false, true);
	}
}

void FlightWindow::namen_eintragen (lbl_cbox* vorname, lbl_cbox *nachname, NamePart quelle, int *anzahl_kandidaten, db_id *kandidaten_id, bool preserve_target_text)
	/*
	 * Read the matching names from the database and write them to the
	 * incomplete lists.
	 * Parameters:
	 *   - vorname: the field containing the first name.
	 *   - nachname: the field containing the last name.
	 *   - quelle: which name part was entered by the user and should be used as
	 *     a source for the search.
	 *   - preserve_target_text: the text in the target field is preserved in
	 *     any case, even if it is empty. Effectively prevents autocompletion.
	 * Set parameters:
	 *   - *anzahl_kandidaten: set to the number of matching names read from the
	 *     database.
	 *   - *kandidaten_id: set to the ID of the match if there is only one
	 *     match. Unspecified else.
	 */
{
	lbl_cbox *quellfeld, *zielfeld;
	int anzahl=0;

	//int (*list_funktion)(QString ***, QString);

	// Zuordnung von (quelle, ziel, funktion) zu (vorname, nachname)
	switch (quelle)
	{
		case nt_vorname:
			quellfeld=vorname;
			zielfeld=nachname;
			break;
		case nt_nachname:
			quellfeld=nachname;
			zielfeld=vorname;
			break;
		default:
			log_error ("Unbehandelter Quellentyp in sk_win_flight::namen_eintragen ()");
			// Fatal. Da /wollen/ wir nicht weitermachen.
			return;
			break;
	}

	QString old_ziel=zielfeld->currentText ();

	QString quelltext=quellfeld->currentText ().stripWhiteSpace ();
	QString zieltext=zielfeld->currentText ().stripWhiteSpace ();

	QStringList names;

	// Ja gut, das ist nat�rlich Pfusch. Die ganze Klasse geh�rt neu
	// geschrieben. Version 2...
#define locked(a) { bool old_locked=lock_edit_slots; lock_edit_slots=true; do {a} while (false); lock_edit_slots=old_locked; }

	if (eintrag_ist_leer (quelltext))
	{
		// L�nge des Quelltexts ist 0. Im Zielfeld alle anzeigen.
		locked (zielfeld->use_full (true);)
		if (anzahl_kandidaten) if (eintrag_ist_leer (zieltext)) *anzahl_kandidaten=0;
	}
	else
	{
		// L�nge des Quelltexts ist nicht gleich 0. Alle die auflisten,
		// die dazu passen, und ins Zielfeld schreiben. Das sollte so
		// wenige sein, dass es schnell geht.

		switch (quelle)
		{
			case nt_vorname:
				db->list_last_names (names, quelltext);
				break;
			case nt_nachname:
				db->list_first_names (names, quelltext);
				break;
			default:
				log_error ("Unbehandelter Quellentyp in sk_win_flight::namen_eintragen () II");
		}

		anzahl=names.count ();
		if (anzahl_kandidaten)
			*anzahl_kandidaten=names.count ();

		if (anzahl==0)
		{
			// Wenn es nichts passendes gibt, alles eintragen.
			locked ( zielfeld->use_full (true); )
		}
		else
		{
			// Wenn es etwas passendes gibt, das eintragen.
			locked
			(
				zielfeld->use_full (false);
				zielfeld->fillStringArray (names, true, true);
			);

			if (kandidaten_id)
			{
				if (anzahl==1)
				{
					QPtrList<Person> persons;
					persons.setAutoDelete (true);
					// TODO error handling
					db->list_persons_by_name (persons, vorname->currentText (), nachname->currentText ());
					// TODO else... error handling, >1 error handling
					if (persons.count ()>0) *kandidaten_id=persons.first ()->id;
				}
			}
		}
	}

	if (preserve_target_text || !old_ziel.isEmpty () || anzahl!=1)
	{
		locked ( zielfeld->setEditText (old_ziel););
	}
}



void FlightWindow::done (int r)
	/*
	 * Closes the dialog and does something, according to the action.
	 * Parameters:
	 *   - r: The action that caused the close:
	 *     0: cancel
	 *     1: (start/land) now
	 *     2: (start/land) later
	 * Members used:
	 *   - flight
	 *   - mode
	 */
{
	if (r>0 && flight)
	{
		// Flug in die Datenbank schreiben.
		// TODO das woanders hin
		if (mode==fe_create) flight->id=0;
		db_id id=db->write_flight (flight);
		if (id_invalid (id))
		{
			log_error ("Fehler beim Eintragen des Flugs in die Datenbank in sk_win_flight::done ()");
		}
		else
		{
			DbEvent event (mode==fe_create?det_add:det_change, db_flug, id);
			emit db_change (&event);
		}
	}
	emit dialog_finished (r);
	QDialog::done (r);
}

void FlightWindow::slot_ok ()
	/*
	 * The OK/((land/start) now) button was pressed. Check if the flight is OK,
	 * then write it to the database and close the window.
	 */
{
	but_ok->setFocus ();

	if (accept_flight_data ())
	{
		FlightMode m=modi[edit_modus->currentItem ()];

		// TODO waaaah diese Bedingung ist doch garantiert nicht vollst�ndig.
		// Au�erdem Codeduplikation mit dem Code, der die buttons zwischen
		// "jetzt/sp�ter starten" und "OK" umschaltet.
		// Das ist nur dann, wenn auch tats�chlich "create now" durchgef�hrt wird.
		// Bei zum Beispiel Nachtragen ist das nicht der Fall.
		// Die aktuelle L�sung ist jedenfalls Murx.
		if (mode==fe_create && edit_datum->date ()==QDate::currentDate ())
		{
			switch (m)
			{
				// PFUSCH: Das hier sollte in accept_flight_data, oder zumindest
				// die Entscheidung, ob jetzt gestartet/gelandet wird.
				case fmLocal: case fmLeaving:
					if (edit_startzeit->checked ()) flight->starten ();
					break;
				case fmComing:
					if (edit_landezeit->checked ()) flight->landen ();
					break;
				default:
					log_error ("Unbehandelter Flugmodus in sk_win_flight::slot_ok ()");
					break;
			}
		}
		done (1);
	}
}

void FlightWindow::slot_later ()
	/*
	 * The start later button was pressed.
	 */
{
	but_later->setFocus ();

	if (accept_flight_data (true))
	{
		done (2);
	}
}

void FlightWindow::slot_cancel ()
	/*
	 * The cancel button was pressed. Close the window.
	 */
{
	done (0);
}



void FlightWindow::enable_widget (int ind, bool en)
	/*
	 * Show or hide a widget, given its index.
	 * Parameters:
	 *   - ind: the index of the widget.
	 *   - en: whether the widget is to be shown (true) or hidden (false).
	 */
{
	// TODO sollte display/show_widget hei�en.

	// We don't hide () and show () the labels because then the layout manager
	// would remove the row along with its spacing so the widgets below would
	// shift up.
	if (en)
	{
		edit_widget[ind]->show ();
		label[ind]->set_invisible (false);
		// TODO immer auch den entsprechenden slot aufrufen
	}
	else
	{
		edit_widget[ind]->hide ();
		label[ind]->set_invisible (true);
	}
}

void FlightWindow::enable_widget (QWidget *wid, bool en)
	/*
	 * Show or hide a widget, given a pointer to it.
	 * Parameters:
	 *   - wid: a pointer to the widget.
	 *   - en: whether the widget is to be shown (true) or hidden (false).
	 */
{
	// Don't show/hide directly but use the index so the corresponding label can
	// also be shown/hidden.
	enable_widget (widget_index (wid), en);
}

void FlightWindow::set_field_error (QWidget *w, bool _error)
	/*
	 * XXX
	 * Parameter:
	 *   - w: the widget of the editing field to set.
	 */
{
	label[widget_index (w)]->set_error (_error);
}



void FlightWindow::slot_registration ()
	/*
	 * Get the selected plane from the database, and, if found, set up some of
	 * the editing fields accordingly.
	 * Members used:
	 *   - selected_plane
	 *   - edit_registration
	 */
{
	if (lock_edit_slots) return;

	if (selected_plane) delete selected_plane;
	selected_plane=new Plane;
	int ret=db->get_plane_registration (selected_plane, edit_registration->currentText ());
	if (ret==db_ok)
	{
		if (id_invalid (startarten[edit_startart->currentItem ()]))
		{
			// Motorflugzeuge machen nur Selbststart, aber nur, wenn
			// nicht was anderes angegeben war.
			if (selected_plane->category==Plane::categorySingleEngine || selected_plane->category==Plane::categoryUltralight)
				edit_startart->setCurrentItem (startart_index (db->get_startart_id_by_type (sat_self)));
		}

		edit_flugzeug_typ->setText (selected_plane->typ);

		if (selected_plane->sitze==1)
		{
			if (eintrag_ist_leer (edit_begleiter_vn->edit_text_string ())) edit_begleiter_vn->setCurrentText ("-");
			if (eintrag_ist_leer (edit_begleiter_nn->edit_text_string ())) edit_begleiter_nn->setCurrentText ("-");
		}
		else
		{
			if (eintrag_ist_leer (edit_begleiter_vn->edit_text_string ())) edit_begleiter_vn->setCurrentText ("");
			if (eintrag_ist_leer (edit_begleiter_nn->edit_text_string ())) edit_begleiter_nn->setCurrentText ("");
		}

		if (flight) flight->flugzeug=selected_plane->id;
	}
	else
	{
		delete selected_plane;
		selected_plane=NULL;
		edit_flugzeug_typ->setText ("");
		if (flight) flight->flugzeug=invalid_id;
	}

	fehler_eintragen (flight, selected_plane, selected_towplane);
}

void FlightWindow::slot_registration_in ()
	/*
	 * Moves the cursor of the registration field to the end. Used because
	 * registration_prefix is preentered.
	 */
{
	if (lock_edit_slots) return;
	// TODO only if registration_prefix, else select the text
	edit_registration->cursor_to_end ();
}

void FlightWindow::slot_flugtyp (int ind)
	/*
	 * Setup controls according to the flight type chosen.
	 * Parameters:
	 *   - ind: the index of the flight type.
	 */
{
	if (lock_edit_slots) return;
	FlightType typ=flightTypes[ind];

	int pilot_idx=widget_index (edit_pilot_nn);
	int begleiter_idx=widget_index (edit_begleiter_nn);
	int towpilot_idx=widget_index (edit_towpilot_nn);

	enable_widget (edit_begleiter_nn, begleiter_erlaubt (typ));
	enable_widget (edit_begleiter_vn, begleiter_erlaubt (typ));

	// TODO remove, wenn bei widget_enable der slot aufgerufen wird
	if (begleiter_erlaubt (typ))
	{
		// Variable f->begleiter zerst�rt
		slot_begleiter_nn ();
		slot_begleiter_vn ();
	}

	label[pilot_idx]->setText ("Pilot");
	label[begleiter_idx]->setText ("Begleiter");
	label[towpilot_idx]->setText ("Schleppilot");

	switch (typ)
	{
		case ftNormal:
			break;
		case ftTraining1:
			label[pilot_idx]->setText ("Flugsch�ler");
			break;
		case ftTraining2:
			label[pilot_idx]->setText ("Flugsch�ler");
			label[begleiter_idx]->setText ("Fluglehrer");
			break;
		case ftGuestPrivate: case ftGuestExternal:
			break;
		case ftTow:
			log_error ("Schleppflug darf nicht ausw�hlbar sein in sk_win_flight::slot_flugtyp ()");
			break;
		default:
			log_error ("Unbehandelter Flugtyp in sk_win_flight::slot_flugtyp ()");
			break;
	}

	if (flight)
	{
		flight->flugtyp=typ;
		if (!begleiter_erlaubt (typ)) flight->begleiter=0;
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_pilot_vn ()
	/*
	 * The (first/last) name of the (pilot/copilot) was entered. Fill the list
	 * for the other part.
	 * Members used:
	 *   - flight
	 *   - edit_(pilot/begleiter)_(vn/nn)
	 */
{
	if (lock_edit_slots) return;
	db_id id;
	namen_eintragen (edit_pilot_vn, edit_pilot_nn, nt_vorname, &anzahl_pilot, &id);
	if (flight)
	{
		if (anzahl_pilot==1) flight->pilot=id;
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_pilot_nn ()
	/*
	 * The (first/last) name of the (pilot/copilot) was entered. Fill the list
	 * for the other part.
	 * Members used:
	 *   - flight
	 *   - edit_(pilot/begleiter)_(vn/nn)
	 */
{
	if (lock_edit_slots) return;
	db_id id;
	namen_eintragen (edit_pilot_vn, edit_pilot_nn, nt_nachname, &anzahl_pilot, &id);
	if (flight)
	{
		if (anzahl_pilot==1) flight->pilot=id;
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_begleiter_vn ()
	/*
	 * The (first/last) name of the (pilot/copilot) was entered. Fill the list
	 * for the other part.
	 * Members used:
	 *   - flight
	 *   - edit_(pilot/begleiter)_(vn/nn)
	 */
{
	if (lock_edit_slots) return;
	db_id id;
	namen_eintragen (edit_begleiter_vn, edit_begleiter_nn, nt_vorname, &anzahl_begleiter, &id);
	if (flight)
	{
		if (anzahl_begleiter==1) flight->begleiter=id;
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_begleiter_nn ()
	/*
	 * The (first/last) name of the (pilot/copilot) was entered. Fill the list
	 * for the other part.
	 * Members used:
	 *   - flight
	 *   - edit_(pilot/begleiter)_(vn/nn)
	 */
{
	if (lock_edit_slots) return;
	db_id id;
	namen_eintragen (edit_begleiter_vn, edit_begleiter_nn, nt_nachname, &anzahl_begleiter, &id);
	if (flight)
	{
		if (anzahl_begleiter==1) flight->begleiter=id;
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_towpilot_vn ()
	/*
	 * The (first/last) name of the (pilot/copilot) was entered. Fill the list
	 * for the other part.
	 * Members used:
	 *   - flight
	 *   - edit_(pilot/begleiter)_(vn/nn)
	 */
{
	if (lock_edit_slots) return;
	db_id id;
	namen_eintragen (edit_towpilot_vn, edit_towpilot_nn, nt_vorname, &anzahl_towpilot, &id);
	if (flight)
	{
		if (anzahl_towpilot==1) flight->towpilot=id;
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_towpilot_nn ()
	/*
	 * The (first/last) name of the (pilot/copilot) was entered. Fill the list
	 * for the other part.
	 * Members used:
	 *   - flight
	 *   - edit_(pilot/begleiter)_(vn/nn)
	 */
{
	if (lock_edit_slots) return;
	db_id id;
	namen_eintragen (edit_towpilot_vn, edit_towpilot_nn, nt_nachname, &anzahl_towpilot, &id);
	if (flight)
	{
		if (anzahl_towpilot==1) flight->towpilot=id;
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_modus (int ind)
	/*
	 * Setup the controls according to the flight mode.
	 * Parameters:
	 *   - ind: the index of the flight mode.
	 */
{
	if (lock_edit_slots) return;
	FlightMode m=modi[ind];

	// TODO: Besser: Wenn noch nicht manuell editiert.
	if (mode==fe_create)
	{
		// TODO Bedingung �berpr�fen
		if (starts_here (m))
		{
			// Leeren Startort auf default setzen
			if (eintrag_ist_leer (edit_startort->currentText ())) edit_startort->setCurrentText (opts.ort);
			// Default Zielort auf "" setzen
			if (eintrag_ist_leer (edit_zielort->currentText ()) || edit_zielort->currentText ()==opts.ort) edit_zielort->setCurrentText ("");
		}
		else
		{
			// Default startort auf "" setzen
			if (eintrag_ist_leer (edit_startort->currentText ()) || edit_startort->currentText ()==opts.ort) edit_startort->setCurrentText ("");
			// Leeren Zielort auf default setzen
			if (eintrag_ist_leer (edit_zielort->currentText ())) edit_zielort->setCurrentText (opts.ort);
		}
	}

	// TODO: den ganzen Kram in setup_controls?
	// den aus den anderen editor slots dann auch
	enable_widget (edit_startart, starts_here (m));
	enable_widget (edit_startzeit, starts_here (m));
	enable_widget (edit_landezeit, lands_here (m));
	if (!starts_here (m))
	{
		// Wenn der Flug von extern kommt, dann ist die Startart sowieso
		// irrelevant, dann m�ssen wir auch nicht drauf achten und k�nnen
		// davon ausgehen, dass kein SFZ vorhanden ist.
		enable_widget (edit_landezeit_sfz, false);
	}
	else
	{
		// Wenn der Flug allerdings nicht von extern kommt, dann kann noch
		// lange nicht automatisch davon ausgegangen werden, dass es ein
		// SFZ gibt --> das h�ngt von der Startart ab.
		slot_startart (edit_startart->currentItem ());
	}
	setup_controls (false);

	if (flight)
	{
		flight->modus=m;
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_startart (int ind)
	/*
	 * Setup the edit fields according to the Startart chosen.
	 * Parameters:
	 *   - ind: the index of the Startart.
	 */
{
	if (lock_edit_slots) return;
	db_id sa_id;
	LaunchType sa;
	if (ind==unknown_startart_index)
	{
		sa_id=original_startart;
	}
	else
	{
		sa_id=startarten[ind];
		db->get_startart (&sa, sa_id);
	}

	// TODO wird das nicht auch schon irgendwo anders gemacht?
	bool towflight_widgets_visible=sa.is_airtow ();
	bool towplane_widgets_visible=sa.is_airtow () && !sa.towplane_known ();
	bool towplane_type_visible=sa.is_airtow ();

	enable_widget (edit_landezeit_sfz, towflight_widgets_visible);
	enable_widget (edit_modus_sfz, towflight_widgets_visible);
	enable_widget (edit_zielort_sfz, towflight_widgets_visible);

	enable_widget (edit_registration_sfz, towplane_widgets_visible);
	enable_widget (edit_typ_sfz, towplane_type_visible);

	enable_widget (edit_towpilot_nn, opts.record_towpilot && towflight_widgets_visible);
	enable_widget (edit_towpilot_vn, opts.record_towpilot && towflight_widgets_visible);

	// TODO immer wenn ein control angezeigt wird, den entsprechenden slot aufrufen (?)
	if (towflight_widgets_visible)
	{
		slot_modus_sfz (edit_modus_sfz->currentItem ());	// TODO currentItem in die Funktion schieben und -1 �bergeben
	}

	if (towplane_widgets_visible)
	{
		slot_registration_sfz ();
	}
	else if (towplane_type_visible)
	{
		if (selected_towplane) delete selected_towplane;
		selected_towplane=new Plane;
		int ret=db->get_plane_registration (selected_towplane, sa.get_towplane ());
		// Not entering this to the flight
		if (ret==db_ok)
		{
			edit_typ_sfz->setText (selected_towplane->typ);
		}
		else
		{
			delete selected_towplane;
			selected_towplane=NULL;
			edit_typ_sfz->setText ("");
		}
	}

	if (flight)
	{
		flight->startart=sa_id;
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_registration_sfz ()
	/*
	 * Get the selected tow plane from the database, and, if found, set up some of
	 * the editing fields accordingly.
	 * Members used:
//	 *   - selected_plane
	 *   - edit_registration_sfz
//	 */
{
	if (lock_edit_slots) return;
	if (selected_towplane) delete selected_towplane;
	selected_towplane=new Plane;
	int ret=db->get_plane_registration (selected_towplane, edit_registration_sfz->currentText ());
	if (ret==db_ok)
	{
		edit_typ_sfz->setText (selected_towplane->typ);
		if (flight) flight->towplane=selected_towplane->id;
	}
	else
	{
		delete selected_towplane;
		selected_towplane=NULL;
		edit_typ_sfz->setText ("");
		if (flight) flight->towplane=invalid_id;
	}

	fehler_eintragen (flight, selected_plane, selected_towplane);
}

void FlightWindow::slot_registration_sfz_in ()
	/*
	 * Moves the cursor of the registration_sfz field to the end. Used because
	 * registration_prefix is preentered.
	 */
{
	if (lock_edit_slots) return;
	// TODO only if registration_prefix, else select the text
	edit_registration_sfz->cursor_to_end ();
}

void FlightWindow::slot_modus_sfz (int ind)
	/*
	 * Setup the controls according to the tow flight mode.
	 * Parameters:
	 *   - ind: the index of the tow flight mode.
	 */
{
	if (lock_edit_slots) return;
	FlightMode m=sfz_modi[ind];

	// Landezeit Schleppflugzeug Text
	// TODO Codeduplikation Text ==> in setup_controls
	if (lands_here (m))
	{
		label[widget_index (edit_landezeit_sfz)]->setText (text_landezeit_schleppflugzeug);
	}
	else
	{
		// TODO Hotkey
		// TODO in setup_controls
		label[widget_index (edit_landezeit_sfz)]->setText (text_schleppende);
	}

	setup_controls (false);

	if (flight)
	{
		flight->modus_sfz=m;
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_gestartet ()
	/*
	 * It was changed if the flight already started. Setup the controls
	 * accordingly.
	 */
{
	if (lock_edit_slots) return;
	if (flight)
	{
		if (mode==fe_edit)
		{
			flight->gestartet=edit_startzeit->time_enabled ();
			fehler_eintragen (flight, selected_plane, selected_towplane);
		}
	}
	edit_startzeit->setFocus ();
}

void FlightWindow::slot_startzeit ()
	/*
	 * The starting time changed. Setup the control accordingly.
	 */
{
	if (lock_edit_slots) return;
	if (flight)
	{
		flight->startzeit.set_to (edit_datum->date (), edit_startzeit->time (), tz_utc, true);
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_gelandet ()
	/*
	 * It was changed if the flight already landed. Setup the controls
	 * accordingly.
	 */
{
	if (lock_edit_slots) return;
	if (flight)
	{
		if (mode==fe_edit)
		{
			flight->gelandet=edit_landezeit->time_enabled ();
			fehler_eintragen (flight, selected_plane, selected_towplane);
		}

		bool time_enabled=edit_landezeit->time_enabled ();
		bool zielort_leer=eintrag_ist_leer (edit_zielort->lineEdit ()->text ());
		bool zielort_default=edit_zielort->currentText ()==opts.ort;
		bool zielort_gleich_startort=(edit_zielort->currentText ()==edit_startort->currentText ());

		// For changing afterwards
		if (time_enabled && zielort_leer)
		{
			//edit_zielort->lineEdit ()->setText (opts.ort);
			edit_zielort->lineEdit ()->setText (edit_startort->lineEdit ()->text ());
			if (eintrag_ist_leer (edit_landungen->text ()))
				edit_landungen->setText ("1");
		}

		if (!time_enabled && (zielort_leer || zielort_default || zielort_gleich_startort))
		{
			edit_zielort->lineEdit ()->setText ("");
		}

		slot_landungen ();
		slot_zielort ();
	}
	edit_landezeit->setFocus ();
}

void FlightWindow::slot_landezeit ()
	/*
	 * The landing time changed. Setup the control accordingly.
	 */
{
	if (lock_edit_slots) return;
	if (flight)
	{
		flight->landezeit.set_to (edit_datum->date (), edit_landezeit->time (), tz_utc, true);
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_sfz_gelandet ()
	/*
	 * It was changed if the towplane already landed. Setup the controls
	 * accordingly.
	 */
{
	if (lock_edit_slots) return;
	if (flight)
	{
		if (mode==fe_edit)
		{
			flight->sfz_gelandet=edit_landezeit_sfz->time_enabled ();
			fehler_eintragen (flight, selected_plane, selected_towplane);
		}

		bool time_enabled=edit_landezeit_sfz->time_enabled ();
		bool zielort_leer=eintrag_ist_leer (edit_zielort_sfz->lineEdit ()->text ());
		bool zielort_default=edit_zielort_sfz->currentText ()==opts.ort;
		bool zielort_gleich_startort=(edit_zielort_sfz->currentText ()==edit_startort->currentText ());

		// For changing afterwards
		if (time_enabled && zielort_leer)
		{
			//edit_zielort_sfz->lineEdit ()->setText (opts.ort);
			edit_zielort_sfz->lineEdit ()->setText (edit_startort->currentText ());
		}

		if (!time_enabled && (zielort_leer || zielort_default || zielort_gleich_startort))
		{
			edit_zielort_sfz->lineEdit ()->setText ("");
		}

		slot_zielort_sfz ();
	}
	edit_landezeit_sfz->setFocus ();
}

void FlightWindow::slot_landezeit_sfz ()
	/*
	 * The landing time of the towplane changed. Setup the control accordingly.
	 */
{
	if (lock_edit_slots) return;
	if (flight)
	{
		flight->landezeit_schleppflugzeug.set_to (edit_datum->date (), edit_landezeit_sfz->time (), tz_utc, true);
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_startort ()
	/*
	 * The starting airfield was changed. Change the controls accordingly.
	 */
{
	if (lock_edit_slots) return;
	if (flight)
	{
		flight->startort=edit_startort->currentText ();
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_zielort ()
	/*
	 * The landing airfield was changed. Change the controls accordingly.
	 */
{
	if (lock_edit_slots) return;
	if (flight)
	{
		flight->zielort=edit_zielort->currentText ();
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_zielort_sfz ()
	/*
	 * The landing airfield of the towplane was changed. Change the controls accordingly.
	 */
{
	if (lock_edit_slots) return;
	// TODO das hier ist Codeduplikation.
	if (flight)
	{
		flight->zielort_sfz=edit_zielort_sfz->currentText ();
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_landungen ()
	/*
	 * The number of landings was changed. Change the controls accordingly.
	 */
{
	if (lock_edit_slots) return;
	if (flight)
	{
		flight->landungen=edit_landungen->text ().toInt ();
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_datum ()
	// ???
{
	if (lock_edit_slots) return;
	if (flight) accept_date ();
	setup_controls ();
}

void FlightWindow::slot_bemerkung ()
	// The comment field was changed
{
	if (lock_edit_slots) return;
	// TODO: fehler_eintragen: wenn startart/... sonstige, sollte ein Kommentar
	// vorhanden sein.
	// TODO: bei Flugzeugen ebenfalls (category)
	if (flight)
	{
		flight->bemerkungen=edit_bemerkungen->text ();
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}

void FlightWindow::slot_abrechnungshinweis ()
	// The billing information field was changed
{
	if (lock_edit_slots) return;
	if (flight)
	{
		flight->abrechnungshinweis=edit_abrechnungshinweis->currentText ();
		fehler_eintragen (flight, selected_plane, selected_towplane);
	}
}



bool FlightWindow::check_flight (db_id *flugzeug_id, db_id *sfz_id, db_id *pilot_id, db_id *begleiter_id, db_id *towpilot_id, bool spaeter, QWidget **error_control)
	/*
	 * Check if a flight seems correct. If not: ask the user. If the user says
	 * that the flight is OK, regard is as correct. Also query the user if there
	 * are multiple persons with the same name.
	 * Parameters:
	 *   - spaeter: if the flight is to be started later. It does not need a
	 *     starting time or a Startart then.
	 * Parameters set:
	 *   - *flugzeug_id: the ID of the plane, if it is found.
	 *   - *sfz_id: the ID of the tow plane, if it is found.
	 *   - *pilot_id: the ID of the pilot, if it is unique.
	 *   - *begleiter_id: the ID of the copilot, if it is unique.
	 *   - *towpilot_id: the ID of the towpilot, if it is unique.
	 *   - *error_control: the first control which contains an erroneous value,
	 *     if any. NULL else.
	 * Members used:
	 *   Lots of edit fields.
	 * Return value:
	 *   - true if the flight is OK
	 *   - false else
	 */
{
	// TODO in diese Funktion sicherstellen, dass alle parameter nur bei
	// existenz geschrieben werden.
	// TODO hier nich today () verwenden
	// TODO alle checkungen mit zeit pr�fen ob ankreuzfeld
	// TODO dazu im time editor funktion, die angibt, ob das timefeld aktiv ist.
	// TODO m�ssten Teile hiervon nicht durch UFC erledigt werden?
	//   - aber nicht alle, es gibt auch Warnungen (Motorflugzeug an der Winde...)
	QString msg;
	bool fz_bekannt=false;
	bool sfz_bekannt=false;

	int startart_index=edit_startart->currentItem ();
	db_id sa_id;

	if (startart_index==unknown_startart_index)
	{
		sa_id=original_startart;
	}
	else
	{
		sa_id=startarten[edit_startart->currentItem ()];
	}

	FlightType typ=flightTypes[edit_flug_typ->currentItem ()];
	Plane fz, sfz;
	// TODO error handling
	LaunchType sa;
	if (!id_invalid (sa_id)) db->get_startart (&sa, sa_id);

	bool kein_begleiter_akzeptiert=false;

	// Es wurde akzeptiert, dass kein Begleiter angegeben ist (bzw. nur Nachname)
	kein_begleiter_akzeptiert=true;

	bool gestartet=edit_startzeit->time_enabled ();
	bool gelandet=edit_landezeit->time_enabled ();
	bool sfz_gelandet=edit_landezeit_sfz->time_enabled ();

	FlightMode modus=modi[edit_modus->currentItem ()];
	FlightMode modus_sfz=sfz_modi[edit_modus_sfz->currentItem ()];

	*error_control=NULL;

	// Keine Startart angegeben
	if (!spaeter && starts_here (modus) && id_invalid (sa_id))
	{
		msg="Es wurde keine Startart angegeben.\n";
		if (error_control) *error_control=edit_startart;
		if (!check_message (this, msg)) return false;
	}

	QString registration=edit_registration->currentText ();

	if (error_control) *error_control=edit_registration;
	if (!check_plane (flugzeug_id, &fz, registration, "Flugzeug", "Flugzeug", (begleiter_id && *begleiter_id>0)?2:0))
		return false;
	else
		fz_bekannt=true;

	bool check_flying=(mode==fe_create) && !gelandet && !spaeter && edit_datum->date ()==QDate::currentDate ();

	if (flugzeug_id && check_flying && !check_plane_flying (*flugzeug_id, fz.registration, "Flugzeug")) return false;

	if (sa.is_airtow ())
	{
		QString towplane_registration;

		if (sa.towplane_known ())
		{
			towplane_registration=sa.get_towplane ();
			if (error_control) *error_control=edit_startart;
		}
		else
		{
			towplane_registration=edit_registration_sfz->currentText ();
			if (error_control) *error_control=edit_registration_sfz;
		}

		// sfz_id is not written if the towplane is known from the startart.
		db_id local_sfz_id=invalid_id;
		if (!check_plane (&local_sfz_id, &sfz, towplane_registration, "Schleppflugzeug", "Schleppflugzeug", 0))
			return false;
		else
			sfz_bekannt=true;

		if (sfz_bekannt && !sa.towplane_known ())
		{
			if (sfz_id) *sfz_id=local_sfz_id;
		}

		if (mode==fe_create && !spaeter && !gelandet && sfz_id && !check_plane_flying (local_sfz_id, sfz.registration, "Schleppflugzeug"))
			return false;

		if (sfz_bekannt && sa.is_airtow () && !sa.towplane_known () && sfz.category==Plane::categoryGlider)
		{
			msg="Laut Datenbank ist das Schleppflugzeug "+sfz.registration+" ("+sfz.typ+") ein Segelflugzeug.\n";
			if (error_control) *error_control=edit_registration_sfz;
			if (!check_message (this, msg)) return false;
		}
	}

	// TODO: andere pr�fung auf fe_create als oben
	// TODO mit check_plane_flying machen

	// Begleiter enth�lt "+1"
	if (begleiter_erlaubt (typ) && (edit_begleiter_vn->currentText ().find (QString ("+1"), 0, false)>=0 || edit_begleiter_nn->currentText ().find (QString ("+1"), 0, false)>=0))
	{
		int ret=QMessageBox::information (this, "Begleiter fehlerhaft",
				"F�r den Begleiter wurde \"+1\" angegeben.\n"
				"F�r Gastfl�ge soll stattdessen der Flugtyp\n"
				"\"Gastflug Privat\" der \"Gastflug Extern\"\n"
				"verwendet werden.\n"
				"Soll dieser Flugtyp verwendet werden?",
				"&Ja", "&Nein", QString::null, 0, 1);
		if (ret==0)
		{
			// Ja, Gastflug
			typ=ftGuestExternal;
			edit_flug_typ->setCurrentItem (flugtyp_index (typ));
			if (begleiter_id) *begleiter_id=invalid_id;
		}
	}

	// Kein Startort angegeben
	if ((gestartet || !starts_here (modus)) && eintrag_ist_leer (edit_startort->currentText ()))
	{
		msg="Es wurde kein Startort angegeben.\n";
		if (error_control) *error_control=edit_startort;
		if (!check_message (this, msg)) return false;
	}

	// Kein Zielort angegeben
	if ((gelandet || !lands_here (modus)) && eintrag_ist_leer (edit_zielort->currentText ()))
	{
		msg="Es wurde kein Zielort angegeben.\n";
		if (error_control) *error_control=edit_zielort;
		if (!check_message (this, msg)) return false;
	}

	// Startort gleich Zielort, wenn es nicht so sein sollte
	if ((starts_here (modus)!=lands_here (modus)) && (edit_zielort->currentText ()==edit_startort->currentText ()))
	{
		msg="Der Startort ist gleich dem Zielort\n";
		if (error_control) *error_control=starts_here (modus)?edit_zielort:edit_startort;
		if (!check_message (this, msg)) return false;
	}


	// Kein Zielort f�r das Schleppflugzeug angegeben
	if ((sfz_gelandet || !lands_here (modus_sfz)) && eintrag_ist_leer (edit_zielort_sfz->currentText ()))
	{
		msg="Es wurde kein Zielort f�r das Schleppflugzeug angegeben.\n";
		if (error_control) *error_control=edit_zielort_sfz;
		if (!check_message (this, msg)) return false;
	}

	// Landung vor Start
	if (gelandet && gestartet && edit_startzeit->time ().secsTo (edit_landezeit->time ())<0 && !edit_landezeit->checked () && !edit_landezeit->time ().isNull ())
	{
		msg="Die Landezeit des Flugs liegt vor der Startzeit.\n";
		if (error_control) *error_control=edit_landezeit;
		if (!check_message (this, msg)) return false;
	}

	// Landung ohne Start
	if (starts_here (modus) && lands_here (modus) && gelandet && !gestartet)
	{
		msg="Es wurde eine Landezeit, aber keine Startzeit angegeben.\n";
		if (error_control) *error_control=edit_landezeit;
		if (!check_message (this, msg)) return false;
	}

	// Gelandet, aber Ladungen==0
	if (lands_here (modus) && gelandet && edit_landungen->text ().toInt ()==0)
	{
		msg="Es wurde eine Landezeit angegeben, aber die Anzahl der Landungen ist 0.\n";
		if (error_control) *error_control=edit_landungen;
		if (!check_message (this, msg)) return false;
	}

	// Landung SFZ vor Start
	if (sfz_gelandet && gestartet && edit_startzeit->time ().secsTo (edit_landezeit_sfz->time ())<0 && !edit_landezeit_sfz->checked () && !edit_landezeit_sfz->time ().isNull ())
	{
		msg="Die Landezeit des Schleppflugs liegt vor der Startzeit.\n";
		if (error_control) *error_control=edit_landezeit_sfz;
		if (!check_message (this, msg)) return false;
	}

	// Landung SFZ ohne Start
	if (sfz_gelandet && !gestartet)
	{
		msg="Es wurde eine Landezeit des Schleppflugs, aber keine Startzeit angegeben.\n";
		if (error_control) *error_control=edit_landezeit_sfz;
		if (!check_message (this, msg)) return false;
	}

	// TODO sollte das nicht fz_bekannt hei�en?
	if (flugzeug_id && *flugzeug_id>0)
	{
		// Gastflug in einsitzigem Flugzeug
		if (fz.sitze==1 && (typ==ftGuestPrivate||typ==ftGuestExternal))
		{
			msg="Laut Datenbank ist das Flugzeug \""+fz.registration+"\" ("+fz.typ+") einsitzig.\n"
				"In einem Einsitzer kann man keinen Gastflug machen.\n";
			if (error_control) *error_control=edit_registration;
			if (!check_message (this, msg)) return false;
		}

		// Doppelsitzige Schulung in einsitzigem Flugzeug
		if (fz.sitze==1 && typ==ftTraining2)
		{
			msg="Laut Datenbank ist das Flugzeug \""+fz.registration+"\" ("+fz.typ+") einsitzig.\n"
					"Es wurde jedoch doppelsitzige Schulung angegeben.\n";
			if (error_control) *error_control=edit_flug_typ;
			if (!check_message (this, msg)) return false;
		}

		// Segelflugzeug mit Zwischenlandungen
		if (fz.category==Plane::categoryGlider && edit_landungen->text ().toInt ()>1 && !sa.is_airtow ())
		{
			msg="Laut Datenbank ist das Flugzeug \""+fz.registration+"\" ("+fz.typ+") ein Segelflugzeug.\n"
					"Es wurden jedoch mehr als eine Landung angegeben.\n";
		if (error_control) *error_control=edit_landungen;
			if (!check_message (this, msg)) return false;
		}

		// Segelflugzeug mit Zwischenlandungen
		if (fz.category==Plane::categoryGlider && !gelandet && edit_landungen->text ().toInt ()>0 && !sa.is_airtow ())
		{
			msg="Laut Datenbank ist das Flugzeug \""+fz.registration+"\" ("+fz.typ+") ein Segelflugzeug.\n"
					"Es wurden jedoch eine Landung, aber keine Landezeit angegeben.\n";
		if (error_control) *error_control=edit_landungen;
			if (!check_message (this, msg)) return false;
		}

		// Segelflugzeug im Selbststart
		if (fz.category==Plane::categoryGlider && sa.get_type ()==sat_self)
		{
			msg="Laut Datenbank ist das Flugzeug \""+fz.registration+"\" ("+fz.typ+") ein Segelflugzeug.\n"
					"Es wurden jedoch \"Selbststart\" als Startart angegeben.\n";
		if (error_control) *error_control=edit_startart;
			if (!check_message (this, msg)) return false;
		}

		// Motorflugzeug an der Winde
		if (fz.category==Plane::categorySingleEngine && sa.get_type ()!=sat_self)
		{
			msg="Laut Datenbank ist das Flugzeug \""+fz.registration+"\" ("+fz.typ+") ein Motorflugzeug.\n"
					"Es wurden jedoch eine andere Startart als Eigenstart angegeben.\n";
		if (error_control) *error_control=edit_startart;
			if (!check_message (this, msg)) return false;
		}
	}

	// Personenkram am Ende pr�fen, weil sonst Personenauswahl nach jedem
	// korrigierten Fehler.
	// Piloten und Begleiter nicht angegeben oder stehen nicht in der Datenbank
	if (error_control) *error_control=edit_pilot_nn;

	QString *preselection_club=NULL;
	// TODO: Vereinheitlichen mit UFC-Code!
	// TODO Modus-Bedingung richtig (erforderlich)
	if (fz_bekannt) preselection_club=&(fz.club);
	// We don't require the name if the flight does not start here or the flight does a self launch.
	if (false==check_person (pilot_id, edit_pilot_vn->currentText (), edit_pilot_nn->currentText (), t_pilot_bezeichnung (typ, cas_nominativ), t_pilot_bezeichnung (typ, cas_akkusativ), (starts_here (modus) && sa.get_type ()!=sat_self), check_flying, original_pilot_id, preselection_club))
		return false;

	if (begleiter_erlaubt (typ))
	{
		// Begleiter ist nie erforderlich. Lehrer wird sp�ter gesondert erfasst.
		if (error_control) *error_control=edit_begleiter_nn;
		// TODO: F�r die preselection sollte man, wenn es keine Person mit dem
		// Verein des Flugsch�lers gibt, doch wieder auf den Verein des
		// Flugzeugs schauen. Das merkt man allerdings erst in check_person,
		// daher m�ssen da beide Vereine reingegeben werden.
		if (typ==ftTraining2 && pilot_id && !id_invalid (*pilot_id))
		{
			// Double seated training. The flight instructor is preselected
			// according to the flight student, if present.
			Person pilot;
			db->get_person (&pilot, *pilot_id);
			preselection_club=&(pilot.club);
		}
		else if (fz_bekannt)
		{
			// Not so. Use the plane instead.
			preselection_club=&(fz.club);
		}

		if (false==check_person (begleiter_id, edit_begleiter_vn->currentText (), edit_begleiter_nn->currentText (), t_begleiter_bezeichnung (typ, cas_nominativ), t_begleiter_bezeichnung (typ, cas_akkusativ), false, check_flying, original_begleiter_id, preselection_club))
			return false;

		// Wenn wir jetzt noch hier sind, dann gibt es entweder einen Begleiter
		// oder es wurde akzeptiert, dass es keinen gibt.
		if (begleiter_id && id_invalid (*begleiter_id)) kein_begleiter_akzeptiert=true;
	}
	else
	{
		// TODO: Hier checken, ob Begleiter angegeben wurde, ggf meckern?
		if (begleiter_id) *begleiter_id=0;
	}


	// Towpilot
	if (false==check_person (towpilot_id, edit_towpilot_vn->currentText (), edit_towpilot_nn->currentText (), "Schleppilot", "Schleppiloten", (opts.record_towpilot && sa.is_airtow ()), check_flying, original_towpilot_id, NULL))
		return false;

	// Pilot und Begleiter identisch
	if (pilot_id && begleiter_id && (*pilot_id)!=0 && (*pilot_id)==(*begleiter_id))
	{
		msg="Pilot und Begleiter sind identisch.\n";
		if (error_control) *error_control=edit_pilot_nn;
		if (!check_message (this, msg)) return false;
	}

	// Pilot und Schleppilot identisch
	if (opts.record_towpilot && pilot_id && towpilot_id && (*pilot_id)!=0 && (*pilot_id)==(*towpilot_id))
	{
		msg="Pilot und Schleppilot sind identisch.\n";
		if (error_control) *error_control=edit_pilot_nn;
		if (!check_message (this, msg)) return false;
	}

	// Kein Lehrer bei doppelsitziger Schulung
	if (typ==ftTraining2 && begleiter_id && id_invalid (*begleiter_id) && !kein_begleiter_akzeptiert)
	{
		msg="Dieser Flug soll eine doppelsitzige Schulung sein.\nEs wurde jedoch kein Lehrer angegeben.\n";
		if (error_control) *error_control=edit_begleiter_nn;
		if (!check_message (this, msg)) return false;
	}

	if (flugzeug_id && *flugzeug_id>0)
	{
		// Begleiter in einsitzigem Flugzeug
		if (fz.sitze==1 && begleiter_id && *begleiter_id!=0)
		{
			msg="Laut Datenbank ist das Flugzeug \""+fz.registration+"\" ("+fz.typ+")\neinsitzig. "
					"Es wurde jedoch ein Begleiter angegeben.\n";
		if (error_control) *error_control=edit_registration;
			if (!check_message (this, msg)) return false;
		}
	}

	return true;
}

bool FlightWindow::person_anlegen (db_id *person_id, QString nachname, QString vorname, QString bezeichnung, bool force)
	/*
	 * Show the person editor and create a person in the database.
	 * Parameters:
	 *   - nachname: the last name of the person to be preentered.
	 *   - vorname: the first name of the person to be preentered.
	 *   - bezeichnung: a description for the person (for example "Pilot")
	 *     (nominativ).
	 *   - force: whether to create the person without asking the user.
	 * Parameters set:
	 *   - *person_id: the ID of the newly created person.
	 */
{
	bool ret=false;
	Person person;		// Puffer zum Schreiben in die Datenbank.

	// Vorname, Nachname eintragen
	person.nachname=nachname;
	person.vorname=vorname;


	// Wenn nicht force: erst nachfragen
	bool aufnehmen;
	// TODO wird das hier uberhaupt gebraucht!?
	if (force)
		aufnehmen=true;
	else
		aufnehmen=(QMessageBox::information (this, "Person nicht bekannt",
		"Der "+bezeichnung+" \""+person.text_name ()+"\" ist nicht bekannt.\nSoll er in die Datenbank aufgenommen werden?",
		"&Ja", "&Nein", QString::null, 0, 1)==0);

	if (aufnehmen)
	{
		EntityEditWindow se (st_person, this, db, "namening", true);
		se.read_db ();
		int erg=se.create (&person, true);

		if (erg==QDialog::Accepted)
		{
			// Person in die Datenbank schreiben
			// TODO error handling
			person.id=0;
			db_id id=db->write_person (&person);
			DbEvent event (det_add, db_person, id);
			emit db_change (&event);
			*person_id=id;
			ret=true;
		}
		else
		{
			ret=false;
		}
	}
	else
	{
		ret=false;
	}

	return ret;
}

void FlightWindow::warning_message (const QString &msg)
	/*
	 * Show a warning message.
	 * Parameters:
	 *   - msg: the message to show.
	 */
{
	QMessageBox::warning (this, "Warnung",
			msg, QMessageBox::Ok, QMessageBox::NoButton);
}

bool FlightWindow::check_plane (db_id *plane_id, Plane *_plane, QString registration, QString description_n, QString description_a, int seat_guess)
	/*
	 * Check if a plane is in the database. If not, it is created after
	 * confirming.
	 * Parameters:
	 *   - registration: the registration
	 *   - description_n: description of the plane, nominative (e. g. "Schleppflugzeug")
	 *   - description_a: description of the plane, accusative (e. g. "Schleppflugzeug")
	 *   - check_flying: Check if the plane is currently flying.
	 *   - seat_guess: the number of seats to preenter in the editor when
	 *     adding the plane; 0 for no guess
	 * Parameters set:
	 *   - *plane_id: the ID of the plane found, if true is returned
	 *   - _plane: the plane is written here, if given and found
	 * Return value:
	 *   - true if accepted
	 *   - false else
	 */
{
	(void)description_a;
	// TODO der Code ist zu einem gewissen Teil identisch mit dem Personencode. Codemerge?

	Plane local_plane;	// MURX: geht nicht auf dem heap weil returns in der Funktion
	Plane *plane=_plane?_plane:&local_plane;

	if (plane_id)
	{
		if (eintrag_ist_leer (registration) || registration==registration_prefix)
		{
			// No registration given.
			QString msg="Es wurde kein "+description_n+" angegeben.\n";
			if (!check_message (this, msg))
				// User aborted
				return false;
			else
				// User accepted
				if (plane_id) *plane_id=0;
		}
		else
		{
			// Registration is given, so the plane must be in the database.

			// Determine plane
			int res=db->get_plane_registration (plane, registration);
			bool plane_known=(res!=db_err_not_found);

			if (plane_known)
			{
				// Plane found, OK
				*plane_id=plane->id;
			}
			else
			{
				// Plane not found
				// Look if there is a plane with registration_prefix in front of the registration
				QString d_registration=registration_prefix+registration;
				bool d_plane_known=(db_ok==db->get_plane_registration (plane, d_registration));
				bool plane_ok=false;

				if (d_plane_known)
				{
					int ret=QMessageBox::information (this, description_n+" nicht bekannt",
							"Das "+description_n+" \""+registration+"\" ist nicht bekannt.\n"
							"Es gibt allerdings ein Flugzeug mit dem Kennzeichen \""+d_registration+"\".\n"
							"Soll dieses Flugzeug verwendet werden?",
							"&Ja", "&Nein", QString::null, 0, 1);
					if (ret==0)
					{
						// Yes, use D-FLUGZEUG.
						if (plane_id) *plane_id=plane->id;
						plane_ok=true;
					}
					else
					{
						// Don't use D-FLUGZEUG
						plane_ok=false;
					}
				}

				if (!plane_ok)
				{
					int ret=QMessageBox::information (this, description_n+" nicht bekannt",
						"Das "+description_n+" \""+registration+"\" ist nicht bekannt.\n"
						"Soll es in die Datenbank aufgenommen werden?",
						"&Ja", "&Nein", QString::null, 0, 1);
					if (ret==0)
					{
						// Yes, create the plane
						plane->registration=registration;
						plane->sitze=seat_guess;
						EntityEditWindow se (st_plane, this, db, "se", true);
						se.read_db ();
						int ret=se.create (plane);

						if (ret==QDialog::Accepted)
						{
							// Plane confirmed
							plane->id=0;
							db_id id=db->write_plane (plane);
//							// Das hier soll helfen, weitere Fehlercheckung zu erm�glichen, wenn FZ aufgenommen.
//							// Aktiveren, wenn OK-Fehlercheckung mit UFC gemacht wird
//							if (selected_plane) delete selected_plane;
//							selected_plane=new Plane (fz);

							DbEvent event (det_add, db_flugzeug, id);
							emit db_change (&event);
							*plane_id=id;
						}
						else
						{
							// Creation aborted
							*plane_id=0;
							return false;
						}
					}
					else
					{
						// No, don't create the plane ==> abort
						*plane_id=0;
						return false;
					}
				}
			}
		}
	}

	return true;
}

bool FlightWindow::check_plane_flying (db_id plane_id, QString registration, QString description_n)
	// registration not determined from plane_id to save time
	// TODO determine registration from plane_id if not given
{
	if (id_invalid (plane_id)) return invalid_id;

	Time ct;
	ct.set_current (true);
	db_id id=db->plane_flying (plane_id, &ct);

	if (!id_invalid (id))
	{
		QString desc;
		if (registration.isEmpty ())
			desc=description_n;
		else
			desc=description_n+" \""+registration+"\"";

		QString msg="Laut Datenbank fliegt das "+desc+" noch (ID: "+QString::number (id)+").\n";
		if (!check_message (this, msg))
			return false;
	}

	return true;
}

bool FlightWindow::check_person (db_id *person_id, QString vorname, QString nachname, QString bezeichnung_n, QString bezeichnung_a, bool person_required, bool check_flying, db_id original_id, QString *preselection_club)
	/*
	 * Check if a person is in the database. If not, it is created after
	 * confirming. If there are multiple persons with that name, show a
	 * selection dialog.
	 * Parameters:
	 *   - vorname: the the first name
	 *   - nachname: the the last name
	 *   - bezeichnung_n: description of the person, nominative (e. g. "Pilot")
	 *   - bezeichnung_a: description of the person, accusative (e. g. "Pilot")
	 *   - person_required: whether the person is required. If true, the user is
	 *     warned if the field is empty.
	 *   - check_flying: Check if the person is currently flying.
	 *   - original_id: The orginal ID of the person, for the case that it
	 *     cannot be identified by the given name(s), but was identified before.
	 *   - preselection_club: The matching club of the plane/flight student/...
	 * Parameters set:
	 *   - *person_id: the ID of the person found, if true is returned
	 * Return value:
	 *   - true if accepted
	 *   - false else
	 */
{
	// Return values
	enum result_type { rt_none, rt_id, rt_no_person, rt_cancel }
	result=rt_none;
	db_id result_id=invalid_id;

	// See which name parts are empty.
	bool vorname_leer=eintrag_ist_leer (vorname);
	bool nachname_leer=eintrag_ist_leer (nachname);

	// Further action to take
	bool display_selector=false;
	bool create_new=false;
	bool confirm_create=true;
	bool confirm=false;	// Whether to confirm first

	// Further action data
	QPtrList<Person> persons; persons.setAutoDelete (true);
	QString action_text;

	// TODO bessere L�sung f�r die Markierung, welche Persone einen passenden
	// Verein haben, und sei es nur, dass da steht "Verein des
	// Flugzeugs"/"Verein des Flugsch�lers"
	QString please_select_text;
	if (preselection_club)
		please_select_text="\nBitte ausw�hlen (Passender Verein: "+*preselection_club+"):";
	else
		please_select_text="\nBitte ausw�hlen:";

	if (vorname_leer && nachname_leer)
	{
		// No name was given.
		// Return "no person". If the person is required, we need to confirm.
		action_text="F�r den Flug wurde kein "+bezeichnung_n+" angegeben.\n";
		result=rt_no_person;
		confirm=person_required;
	}
	else if (vorname_leer && !nachname_leer)
	{
		// Only a last name was given.
		// Display a list of matching first names.
		db->list_persons_by_last_name (persons, nachname);
			action_text="F�r den "+bezeichnung_a+" wurde nur ein Nachname angegeben."+please_select_text;
		display_selector=true;
	}
	else if (!vorname_leer && nachname_leer)
	{
		// Only a first name was given.
		// Display a list of matching last names.
		db->list_persons_by_first_name (persons, vorname);
			action_text="F�r den "+bezeichnung_a+" wurde nur ein Vorname angegeben."+please_select_text;
		display_selector=true;
	}
	else
	{
		// A full name was given.
		// Get the matching names from the database.
		int ret=db->list_persons_by_name (persons, vorname, nachname);
		int num_persons=persons.count ();
		if (ret<0)
		{
			// An error occured.
			// Conservative action: return no person.
			// TODO error message
			log_error ("Fehler bei der Personenabfrage in sk_win_flight::check_person ()");
			result=rt_no_person;
		}
		else if (num_persons==0)
		{
			// There were no persons.
			// Create a database entry.
			create_new=true;
			confirm_create=true;
		}
		else if (num_persons==1)
		{
			// The person is unique.
			// Return it.
			result=rt_id;
			result_id=persons.first ()->id;
		}
		else
		{
			// There was more than one person.
			// Display a selector.
				action_text="Es gibt mehrere Personen mit diesem Namen."+please_select_text;
			display_selector=true;
		}
	}

	// If we need, display the selector
	if (display_selector)
	{
		// There were multiple persons. Let the user select one.
		db_id preselect_id=invalid_id;
		QString simplified_preselection_club;
		if (preselection_club) simplified_preselection_club=simplify_club_name (*preselection_club);
		int num_club_matches=0;
		db_id club_match_id=invalid_id;

		for (QPtrListIterator<Person> person (persons); *person; ++person)
		{
			// Iterate over the persons. If a person with the correct ID is
			// found, this ID is preselected.
			if (!id_invalid (original_id) && (*person)->id==original_id)
			{
				// The original person is in the list. Use that person.
				preselect_id=(*person)->id;
				// Stop processing here because a match in the original ID
				// overrides all other criteria.
				break;
			}

			if (preselection_club && simplify_club_name ((*person)->club)==simplified_preselection_club)
			{
				// The club of this person matches the preselection club.
				club_match_id=(*person)->id;
				num_club_matches++;
			}
		}

		if (num_club_matches==1)
		{
			// The person with matching club is only selected if there
			// was exactly one. If there were more than one, we can't
			// tell which one to use and we don't want to pretend to
			// the user that we known the right person.
			preselect_id=club_match_id;
		}

		EntitySelectWindow<Person> selector (this, "selector");
		selection_result res=selector.do_selection ("Personenauswahl", action_text, persons, preselect_id);

		switch (res)
		{
			case sr_cancelled: case sr_none_selected:
				// Cancelled: cancel the accepting.
				result=rt_cancel;
				break;
			case sr_unknown:
				// Unknown person: only save the part of the name given.
				result=rt_no_person;
				break;
			case sr_new:
				// Create a new person.
				create_new=true;
				confirm_create=false;
				break;
			case sr_ok:
				// Person selected.
				result_id=selector.get_result_id ();
				result=rt_id;
				break;
			default:
				// Unhandled. Conservative action: continue
				log_error ("Unhandled selection result in sk_win_flight::check_person ()");
				break;
		}
	}

	// If we need, create a new person
	if (create_new)
	{
		// The person need to be created, either because the
		if (person_anlegen (&result_id, nachname, vorname, bezeichnung_n, !confirm_create))
			result=rt_id;
		else
			result=rt_cancel;
	}

	// Check if the person is still flying.
	if (result==rt_id && !id_invalid (result_id) && check_flying)
	{
		Time ct;
		ct.set_current (true);
		if (!id_invalid (db->person_flying (result_id, &ct)))
		{
			Person p;
			db->get_person (&p, result_id);
			action_text="Laut Datenbank fliegt der "+bezeichnung_n+" \""+p.text_name ()+"\" noch.\n";
			confirm=true;
		}
	}

	// If the result needs to be confirmed, do so.
	// The result 'cancel' does not need to be confirmed because it is what is
	// done when confirmation fails.
	if (result!=rt_cancel && confirm)
	{
		if (!check_message (this, action_text)) result=rt_cancel;
	}

	// Return something, based on the return type
#define return_with(return_value, return_id) if (person_id) *person_id=return_id; return return_value; break;
	switch (result)
	{
		case rt_cancel:
			return_with (false, invalid_id);
		case rt_id:
			return_with (true, result_id);
		case rt_no_person:
			return_with (true, invalid_id);
		case rt_none:
			log_error ("No result in sk_win_flight::check_person ()");
			return_with (true, invalid_id);
		default:
			log_error ("Unhandled result type in sk_win_flight::check_person ()");
			return_with (true, invalid_id);
	}
#undef return_with

	return true;
}

void FlightWindow::accept_date ()
	// ???
{
	bool gestartet=false, gelandet=false, sfz_gelandet=false;
	// PFUSCH: Das ist schlecht, dass hier unterschieden werden muss. Besser:
	// checked () XOR invert (), aber aufpassen mit setup_controls
	if (mode==fe_edit)
	{
		gestartet=edit_startzeit->checked ();
		gelandet=edit_landezeit->checked ();
		sfz_gelandet=edit_landezeit_sfz->checked ();
	}
	else if (mode==fe_create)
	{
		gestartet=!edit_startzeit->checked ();
		gelandet=!edit_landezeit->checked ();
		sfz_gelandet=!edit_landezeit_sfz->checked ();
	}
	else
	{
		log_error ("Unbekannter modus in sk_win_flight::accept_flight_date ()");
	}

	// MURX: Besser nicht nachtr�glich �berschreiben
	if (!starts_here (flight->modus)) gestartet=false;
	if (!lands_here (flight->modus)) gelandet=false;

	set_time (gestartet, &(flight->gestartet), &(flight->startzeit), edit_datum->date (), edit_startzeit->time ());
	set_time (gelandet, &(flight->gelandet), &(flight->landezeit), edit_datum->date (), edit_landezeit->time ());
	set_time (sfz_gelandet, &(flight->sfz_gelandet), &(flight->landezeit_schleppflugzeug), edit_datum->date (), edit_landezeit_sfz->time ());
}

bool FlightWindow::accept_flight_data (bool spaeter)
	/*
	 * Check the flight data, if OK, write to the flight member variable.
	 * Parameters:
	 *   - spaeter: if the flight is to be started later
	 * Return value:
	 *   - true on accepted
	 *   - false else
	 */
{
	db_id flugzeug_id=invalid_id, sfz_id=invalid_id;
	db_id pilot_id=invalid_id, begleiter_id=invalid_id, towpilot_id=invalid_id;

	// MURX, so that the pilot preselection works
	//pilot_id=flight->pilot;
	//begleiter_id=flight->begleiter;
	//cout << "foo: " << pilot_id << " " << begleiter_id << endl;

	QWidget *error_control=NULL;

	if (check_flight (&flugzeug_id, &sfz_id, &pilot_id, &begleiter_id, &towpilot_id, spaeter, &error_control))
	{
		FlightMode m=modi[edit_modus->currentItem ()];
		FlightMode m_sfz=sfz_modi[edit_modus_sfz->currentItem ()];
		FlightType typ=flightTypes[edit_flug_typ->currentItem ()];

		flight->flugzeug=flugzeug_id;
		flight->flugtyp=typ;
		flight->startort=edit_startort->currentText ();
		flight->zielort=edit_zielort->currentText ();
		flight->zielort_sfz=edit_zielort_sfz->currentText ();
		flight->landungen=edit_landungen->text ().toInt ();

		// pilot_id und begleiter_id k�nnen 0 sein, wenn nur ein Namensteil
		// angegeben und vom Benutzer akzeptiert wurde.
		// begleiter_id kann auch 0 sein, wenn einfach kein Begleiter angegeben
		// wurde.
		// TODO Codeduplikation Pilot, Begleiter, Schleppilot
		if (pilot_id>0)
		{
			// Pilot angegeben und erkannt.
			flight->pilot=pilot_id;
		}
		else
		{
			if (!eintrag_ist_leer (edit_pilot_nn->currentText ()))
				flight->pnn=edit_pilot_nn->currentText ();
			if (!eintrag_ist_leer (edit_pilot_vn->currentText ()))
				flight->pvn=edit_pilot_vn->currentText ();
			flight->pilot=0;

		}

		// TODO ist begleiter_erlaubt das einzige/richtige Kritierium?
		// TODO Vielleicht besser bei !begleiter_erlaubt so tun, als ob die
		// Felder leer w�ren, aber hier nicht �berschreiben.
		if (begleiter_id>0)
		{
			// Begleiter angegeben und erkannt.
			flight->begleiter=begleiter_id;
		}
		else
		{
			if (begleiter_erlaubt (typ) && !eintrag_ist_leer (edit_begleiter_nn->currentText ()))
				flight->bnn=edit_begleiter_nn->currentText ();
			if (begleiter_erlaubt (typ) && !eintrag_ist_leer (edit_begleiter_vn->currentText ()))
				flight->bvn=edit_begleiter_vn->currentText ();

			flight->begleiter=0;
		}

		if (towpilot_id>0)
		{
			// Schlepppilot angegeben und erkannt.
			flight->towpilot=towpilot_id;
		}
		else
		{
			if (!eintrag_ist_leer (edit_towpilot_nn->currentText ()))
				flight->tpnn=edit_towpilot_nn->currentText ();
			if (!eintrag_ist_leer (edit_towpilot_vn->currentText ()))
				flight->tpnn=edit_towpilot_vn->currentText ();
			flight->towpilot=0;
		}


		int startart_index=edit_startart->currentItem ();
		if (startart_index==unknown_startart_index)
			flight->startart=original_startart;
		else
			flight->startart=startarten[startart_index];

		if (!id_invalid (sfz_id)) flight->towplane=sfz_id;
		flight->modus=m;
		flight->modus_sfz=m_sfz;
		flight->bemerkungen=edit_bemerkungen->text ();
		flight->abrechnungshinweis=edit_abrechnungshinweis->currentText ();

		// Datum in alle Felder eintragen
		accept_date ();

		return true;
	}
	else
	{
		// TODO: wenn lineedit, selektion.
		if (error_control) error_control->setFocus ();
		return false;
	}
}

void FlightWindow::set_time (bool use_time, bool *use_ziel, Time *zeit_ziel, QDate datum, QTime zeit)
	// ???
{
	(*use_ziel)=use_time;
	if (use_time)
		zeit_ziel->set_to (datum, zeit, tz_utc, true);
		// Ansonsten: nicht �ndern
//	else
//		zeit_ziel->set_null ();
}

void FlightWindow::flug_eintragen (Flight *f, bool repeat)
	/*
	 * Write the flight data to the editor controls.
	 * Parameters:
	 *   - *f: The flight to be entered
	 *   - repeat: Whether we are editing a flight, as opposed to
	 *     editing it
	 */
{
	// TODO is this function complete? How to ensure this?
	Plane fz;
	if (f->flugzeug>0 && db->get_plane (&fz, f->flugzeug)<0) log_error ("Flugzeug nicht gefunden in sk_win_flight::edit_flight ()");

	Person pilot, begleiter, towpilot;

	original_pilot_id=f->pilot;
	original_begleiter_id=f->begleiter;
	original_towpilot_id=f->towpilot;

	bool old_disable_error_check=disable_error_check;
	disable_error_check=true;

	LaunchType startart;
	bool startart_ok=(db->get_startart (&startart, f->startart)==db_ok);

	// Do this even if the startart is known and is no airtow, because the user
	// might switch to "unknown airtow".
	// If this no airtow, the fields are hidden, so the user won't notice.
	if (!id_invalid (f->towplane))
	{
		Plane tow_plane;
		if (db_ok==db->get_plane (&tow_plane, f->towplane))
		{
			edit_registration_sfz->setCurrentText (tow_plane.registration);
			edit_typ_sfz->setText (tow_plane.typ);
		}
	}

	if (repeat)
	{
		edit_landungen->setText ("");

		edit_startzeit->null_zeit ();
		edit_landezeit->null_zeit ();
		edit_landezeit_sfz->null_zeit ();

		edit_startzeit->set_checked (true);
		edit_landezeit->set_checked (true);
		edit_landezeit_sfz->set_checked (true);
	}
	else
	{
		edit_zielort_sfz->setCurrentText (f->zielort_sfz);
		int mi=sfz_modus_index (f->modus_sfz);
		if (mi<0) mi=modus_index (fmLocal);
		edit_modus_sfz->setCurrentItem (mi);

		edit_landungen->setText (f->landungen>0?QString::number(f->landungen):"");

		// Zeit immer eintragen, auch, wenn nicht aktiv
		edit_startzeit->set_time (f->startzeit.get_qtime ());
		edit_landezeit->set_time (f->landezeit.get_qtime ());
		edit_landezeit_sfz->set_time (f->landezeit_schleppflugzeug.get_qtime ());

		edit_startzeit->set_checked (f->gestartet);
		edit_landezeit->set_checked (f->gelandet);
		edit_landezeit_sfz->set_checked (f->sfz_gelandet);
	}

	// TODO Codeduplikation Pilot/Begleiter/Towpilot
	if (id_invalid (f->pilot))
	{
		edit_pilot_nn->setCurrentText (f->pnn);
		edit_pilot_vn->setCurrentText (f->pvn);
	}
	else
	{
		db->get_person (&pilot, f->pilot);
		edit_pilot_vn->setCurrentText (pilot.vorname);
		edit_pilot_nn->setCurrentText (pilot.nachname);
	}

	if (id_invalid (f->begleiter))
	{
		edit_begleiter_nn->setCurrentText (f->bnn);
		edit_begleiter_vn->setCurrentText (f->bvn);
	}
	else
	{
		db->get_person (&begleiter, f->begleiter);
		edit_begleiter_vn->setCurrentText (begleiter.vorname);
		edit_begleiter_nn->setCurrentText (begleiter.nachname);
	}

	if (id_invalid (f->towpilot))
	{
		edit_towpilot_nn->setCurrentText (f->tpnn);
		edit_towpilot_vn->setCurrentText (f->tpvn);
	}
	else
	{
		db->get_person (&towpilot, f->towpilot);
		edit_towpilot_vn->setCurrentText (towpilot.vorname);
		edit_towpilot_nn->setCurrentText (towpilot.nachname);
	}

	// Felder eintragen, abh�ngig von repeat
	// Hier besteht die Wichtigkeit, dass das nach den Piloten geschieht, weil
	// n�mlich sonst via slot_flugtyp in slot_begleiter_vn/nn der Begleiter in
	// f �berschrieben wird, was auf das unglaublich schlechtes Design dieser
	// Klasse zur�ckzuf�hren ist.
	edit_registration->setCurrentText (fz.registration);
	edit_flugzeug_typ->setText (fz.typ);
	edit_flug_typ->setCurrentItem (flugtyp_index (f->flugtyp));
	edit_startort->setCurrentText (f->startort);
	edit_zielort->setCurrentText (f->zielort);


	// Comments are not copied when repeating a flight.
	if (!repeat) edit_bemerkungen->setText (f->bemerkungen);

	edit_abrechnungshinweis->setCurrentText (f->abrechnungshinweis);

	// Launch types are not copied when repeating a flight.
	// This is because it could be different than before (different winch)
	// An exception is self start because it is sufficiently unlikely that a
	// plane which did a self start does something else the next time.
	if (!repeat || (startart_ok && startart.get_type ()==sat_self))
	{
		// If the startart is OK or there is no startart (id==invalid_id), do
		// not display the "unknown" entry.
		if (startart_ok || id_invalid (f->startart))
		{
			// If there is an "unknown" entry in the Startart list, remove it.
			if (unknown_startart_index>=0)
			{
				edit_startart->removeItem (unknown_startart_index);
				unknown_startart_index=-1;
			}

			// Select the correct Startart list entry.
			// Note: f->startart could have been overwritten by slot_startart
			// because f is the same as the global flight pointer.
			// TODO remove this problem, if possible. Majorly redesign
			// interface and state variables. Among others: slot_* ()
			edit_startart->setCurrentItem (startart_index (startart.get_id ()));
		}
		else
		{
			// If there is no "unknown" entry in the Startart list, create it.
			original_startart=f->startart;

			if (unknown_startart_index<0)
			{
				unknown_startart_index=edit_startart->count ();
				edit_startart->insertItem (unknown_startart_text, unknown_startart_index);
			}

			// Select the "unknown" entry
			edit_startart->setCurrentItem (unknown_startart_index);
		}
	}
	else
	{
		edit_startart->setCurrentItem (invalid_id);
	}

	int mi=modus_index (f->modus);
	if (mi<0) mi=modus_index (fmLocal);
	// BUG: Der folgende Befehl �berschreibe selected_plane
	edit_modus->setCurrentItem (mi);

	if (!repeat)
	{
		int m_sfz_i=sfz_modus_index (f->modus_sfz);
		if (m_sfz_i<0) m_sfz_i=sfz_modus_index (fmLocal);
		edit_modus_sfz->setCurrentItem (m_sfz_i);
	}

	if (f->startzeit.unix_time ()!=0)
		edit_datum->setDate (f->startzeit.get_qdate ());
	else if (f->landezeit.unix_time ()!=0)
		edit_datum->setDate (f->landezeit.get_qdate ());
	else
		edit_datum->setDate (QDate::currentDate ());	// TODO beser kein QDate::currentDate ();

	namen_eintragen (edit_pilot_vn, edit_pilot_nn, nt_vorname, &anzahl_pilot, NULL, true);
	namen_eintragen (edit_pilot_vn, edit_pilot_nn, nt_nachname, &anzahl_pilot, NULL, true);
	namen_eintragen (edit_begleiter_vn, edit_begleiter_nn, nt_vorname, &anzahl_begleiter, NULL, true);
	namen_eintragen (edit_begleiter_vn, edit_begleiter_nn, nt_nachname, &anzahl_begleiter, NULL, true);
	namen_eintragen (edit_towpilot_vn, edit_towpilot_nn, nt_vorname, &anzahl_begleiter, NULL, true);
	namen_eintragen (edit_towpilot_vn, edit_towpilot_nn, nt_nachname, &anzahl_begleiter, NULL, true);

	disable_error_check=old_disable_error_check;
}

void FlightWindow::fehler_eintragen (Flight *f, Plane *fz, Plane *sfz, bool move_focus)
	/*
	 * Show the errors of a flight in the error list control.
	 * Parameters:
	 *   - f: the flight to be checked
	 *   - fz: the plane that is used in the flight
	 */
{
	if (disable_error_check) return;

	for (int i=0; i<NUM_FIELDS; i++) label[i]->set_error (false);

	if (mode==fe_edit)
	{
		int i=0;
		int anzahl=0;
		FlightError ff;
		// TODO error handling
		LaunchType sa; db->get_startart (&sa, f->startart);
		bool schlepp=sa.is_airtow ();

		edit_fehler->clear ();
		int first_erroneous_widget=-1;

		while ((ff=f->fehlerchecking (&i, true, schlepp, fz, sfz, &sa))!=ff_ok)
		{
			if (!(
				(ff==ff_schulung_ohne_begleiter && anzahl_begleiter>=1)||
				(ff==ff_pilot_gleich_begleiter && edit_pilot_vn->currentText ()!=edit_begleiter_vn->currentText ())||
				(ff==ff_pilot_gleich_begleiter && edit_pilot_nn->currentText ()!=edit_begleiter_nn->currentText ())||
				(ff==ff_pilot_gleich_towpilot && edit_pilot_vn->currentText ()!=edit_towpilot_vn->currentText ())||
				(ff==ff_pilot_gleich_towpilot && edit_pilot_nn->currentText ()!=edit_towpilot_nn->currentText ())||
				(ff==ff_keine_startart && unknown_startart_index>=0 && edit_startart->currentItem ()==unknown_startart_index)||
				false))
			{
				edit_fehler->addItem (f->fehler_string (ff));
				QWidget *error_widget=get_error_control (ff);
				if (error_widget)
				{
					int error_widget_index=widget_index (error_widget);
					if (first_erroneous_widget<0 || error_widget_index<first_erroneous_widget) first_erroneous_widget=error_widget_index;
					set_field_error (error_widget, true);
				}
				anzahl++;
			}
		}

		if (move_focus && first_erroneous_widget>=0) edit_widget[first_erroneous_widget]->setFocus ();

		if (anzahl==0) edit_fehler->addItem ("Keine");

		if (anzahl==0)
		{
			edit_fehler->setPalette (palette ());
		}
		else
		{
			QPalette pal=edit_fehler->palette ();
			pal.setColor (QPalette::Base, QColor (255, 127, 127));
			edit_fehler->setPalette (pal);
		}
	}
}



int FlightWindow::go (flight_editor_mode m, Flight *vorlage, QDate *date_to_use)
	/*
	 * The main function to open the flight editor. Does some preparation, then
	 * shows the window.
	 * Parameters:
	 *   - m: the mode (create, edit, ...).
	 *   - *vorlage: the flight to edit, the template for a repetition, NULL
	 *     for a new creation.
	 * Return value:
	 *   - the result of the dialog
	 */
{
	// ATTENTION:
	// the order of reset(), flight=new..., setup_controls() etc. here is quite
	// a mess and was determined from debugging session (read: experimentation).
	// Change with care.
	// If there are problems, try moving reset () befor flight=..., although
	// this might as well break things.
	//
	// TODO the whole invoke-this-window code is a bit of a mess. Rewrite.
	// setup_controls must be called after the flight is created because it
	// also sets selected_plane.
	// TODO: change bad style, maybe move to flug_eintragen
	mode=m;

//	bool skip_date=(date_to_use!=NULL);

	if (flight) delete flight; flight=NULL;
	switch (mode)
	{
		case fe_create:
			if (vorlage)
			{
				// Repeat flight
				flight=new Flight (*vorlage);
				//reset ();	// Overwrites Entity in flight
				setup_controls (true, false, true);
				flug_eintragen (flight, true);
			}
			else
			{
				// Create new flight
				flight=new Flight;
				reset ();
				setup_controls (true, false, false);
			}

			if (date_to_use) edit_datum->setDate (*date_to_use);
			break;
		case fe_edit:
			if (vorlage)
			{
				// Edit or display a flight
				reset ();
				flight=new Flight (*vorlage);
				//reset ();	// Overwrites Entity in flight
				setup_controls (true, !vorlage->editierbar);
				flug_eintragen (flight, false);
				fehler_eintragen (flight, selected_plane, selected_towplane, true);
			}
			else
			{
				log_error ("No flight passed for editing in sk_win_flight::go ()");
				return -1;
			}
			break;
		default:
			log_error ("Unhandled editor mode in sk_win_flight::go ()");
			return -1;
			break;
	}

	if (flugeditor_modal)
  	return exec ();
	else
		show ();

	return 0;
}



int FlightWindow::startart_index (db_id sa)
	/*
	 * For a given Startart, finds its index in the Startart array.  Needed for
	 * setting the Startart editor widget to a given value.
	 * Parameters:
	 *   - sa: the Startart.
	 * Return value:
	 *   - the index of sa in the array, if found.
	 *   - -1 else.
	 */
{
	if (startarten.count ()==1)
	{
		// If there is only one startart, return the first one (there is no
		// "invalid" startart in this case (some functions ask for this
		// startart)).
		return 0;
	}
	else
	{
		// If there are multiple startarten, search for the appropriate one.
		for (int i=0; i<startarten.count (); i++)
		{
			if (startarten[i]==sa) return i;
		}
		log_error ("Startart nicht gefunden in startart_index ()");
		return -1;
	}
}

int FlightWindow::modus_index (FlightMode sa)
	/*
	 * For a given flight mode, finds its index in the flight mode array.
	 * Needed for setting the flight mode editor widget to a given value.
	 * Parameters:
	 *   - sa: the flight mode.
	 * Return value:
	 *   - the index of sa in the array, if found.
	 *   - -1 else.
	 */
{
	for (int i=0; i<num_modi; i++)
	{
		if (modi[i]==sa) return i;
	}
	log_error ("Modus nicht gefunden in modus_index ()");
	return -1;
}

int FlightWindow::sfz_modus_index (FlightMode sa)
	/*
	 * For a given towflight mode, finds its index in the towflight mode array.
	 * Needed for setting the towflight mode editor widget to a given value.
	 * Parameters:
	 *   - sa: the towflight mode.
	 * Return value:
	 *   - the index of sa in the array, if found.
	 *   - -1 else.
	 */
{
	for (int i=0; i<num_sfz_modi; i++)
	{
		if (sfz_modi[i]==sa) return i;
	}
	log_error ("Modus nicht gefunden in sfz_modus_index ()");
	return -1;
}

int FlightWindow::flugtyp_index (FlightType t)
	/*
	 * For a given flight type, finds its index in the flight type array.
	 * Needed for setting the flight type editor widget to a given value.
	 * Parameters:
	 *   - t: the flight type.
	 * Return value:
	 *   - the index of t in the array, if found.
	 *   - -1 else.
	 */
{
	for (int i=0; i<num_flugtypen; i++)
		if (flightTypes[i]==t) return i;

	log_error ("Flugtyp nicht gefunden in flugtyp_index ()");
	return -1;
}

int FlightWindow::widget_index (QWidget *w)
	/*
	 * Finds the index of a given editor widget in the widget array.
	 * Parameters:
	 *   - w: the widget to search for.
	 * Return value:
	 *   - the index of w in the array.
	 *   - -1 if not found.
	 */
{
	for (int i=0; i<NUM_FIELDS; i++)
		if (w==edit_widget[i])
			return i;

	log_error ("Widget nicht gefunden in widget_index ()");
	return -1;
}



void FlightWindow::slot_db_update (DbEvent  *event)
	/*
	 * Handler for the db_change mechanism. See db_change mechanism.
	 */
{
	if (event->type==det_refresh)
		read_db ();

	if ((event->type==det_add||event->type==det_change) && event->id>0)
	{
		if (event->table==db_person)
		{
			Person pn;
			if (db->get_person (&pn, event->id)>=0)
			{
				edit_pilot_nn->insert_full_if_new (pn.nachname);
				edit_pilot_vn->insert_full_if_new (pn.vorname);
				edit_begleiter_nn->insert_full_if_new (pn.nachname);
				edit_begleiter_vn->insert_full_if_new (pn.vorname);
				edit_towpilot_nn->insert_full_if_new (pn.nachname);
				edit_towpilot_vn->insert_full_if_new (pn.vorname);
			}
		}
		if (event->table==db_flugzeug)
		{
			Plane fz;
			if (db->get_plane (&fz, event->id)>=0)
			{
				edit_registration->insert_if_new (fz.registration);
			}
		}
		if (event->table==db_flug)
		{
			Flight fg;
			if (db->get_flight (&fg, event->id)>=0)
			{
				edit_startort->insert_if_new (fg.startort);
				edit_startort->insert_if_new (fg.zielort);
				edit_zielort->insert_if_new (fg.startort);
				edit_zielort->insert_if_new (fg.zielort);
				edit_abrechnungshinweis->insert_if_new (fg.abrechnungshinweis);
			}
		}
	}

	SkDialog::slot_db_update (event);
}



// Interface
int FlightWindow::create_flight (QDate *date_to_use)
	/*
	 * Open the window for creating a flight.
	 * Return value:
	 *   the result of the dialog
	 */
{
	return go (fe_create, NULL, date_to_use);
}

int FlightWindow::edit_flight (Flight *f)
	/*
	 * Open the window for editing a flight.
	 * Parameters:
	 *   - f: the flight to be edited.
	 * Return value:
	 *   the result of the dialog
	 */
{
	return go (fe_edit, f, NULL);
}

int FlightWindow::duplicate_flight (Flight *vorlage)
	/*
	 * Open the window for creating a flight, using a template (for repeating a
	 * flight).
	 * Parameters:
	 *   - vorlage: the flight to be used as a template.
	 * Return value:
	 *   the result of the dialog
	 */
{
	return go (fe_create, vorlage, NULL);
}


// Fehlerbehandlung
QWidget *FlightWindow::get_error_control (FlightError error)
{
	switch (error)
	{
		case ff_keine_id: return NULL;
		case ff_ok: return NULL;
		case ff_kein_flugzeug: return edit_registration;
		case ff_kein_pilot: return edit_pilot_nn;
		case ff_pilot_gleich_begleiter: return edit_begleiter_nn;
		case ff_pilot_nur_vorname: return edit_pilot_nn;
		case ff_pilot_nur_nachname: return edit_pilot_vn;
		case ff_pilot_nicht_identifiziert: return edit_pilot_nn;
		case ff_begleiter_nur_vorname: return edit_begleiter_nn;
		case ff_begleiter_nur_nachname: return edit_begleiter_vn;
		case ff_begleiter_nicht_identifiziert: return edit_begleiter_nn;
		case ff_schulung_ohne_begleiter: return edit_begleiter_nn;
		case ff_begleiter_nicht_erlaubt: return edit_begleiter_nn;
		case ff_nur_gelandet: return edit_landezeit;
		case ff_landung_vor_start: return edit_landezeit;
		case ff_keine_startart: return edit_startart;
		case ff_kein_modus: return edit_modus;
		case ff_kein_sfz_modus: return edit_modus_sfz;
		case ff_kein_flugtyp: return edit_flug_typ;
		case ff_landungen_negativ: return edit_landungen;
		case ff_doppelsitzige_schulung_in_einsitzer: return edit_begleiter_nn;
		case ff_kein_startort: return edit_startort;
		case ff_kein_zielort: return edit_zielort;
		case ff_kein_zielort_sfz: return edit_zielort_sfz;
		case ff_segelflugzeug_landungen: return edit_landungen;
		case ff_begleiter_in_einsitzer: return edit_begleiter_nn;
		case ff_gastflug_in_einsitzer: return edit_flug_typ;
		case ff_segelflugzeug_selbststart: return edit_startart;
		case ff_schlepp_nur_gelandet: return edit_landezeit_sfz;
		case ff_schlepp_landung_vor_start: return edit_landezeit_sfz;
		case ff_landungen_null: return edit_landungen;
		case ff_landungen_ohne_start: return edit_landungen;
		case ff_segelflugzeug_landungen_ohne_landung: return edit_landungen;
		case ff_startort_gleich_zielort: return edit_zielort;
		case ff_kein_schleppflugzeug: return edit_registration_sfz;
		case ff_towplane_is_glider: return edit_registration_sfz;
		case ff_pilot_gleich_towpilot: return edit_pilot_nn;
		case ff_towpilot_nur_vorname: return edit_towpilot_nn;
		case ff_towpilot_nur_nachname: return edit_towpilot_vn;
		case ff_towpilot_nicht_identifiziert: return edit_towpilot_nn;
	}

	return NULL;
}



QColor FlightWindow::get_default_color (QWidget *w)
{
	if (w==edit_registration) return col_obligatory;
	if (w==edit_pilot_nn) return col_obligatory;
	if (w==edit_begleiter_nn) return col_obligatory;
	if (w==edit_towpilot_nn) return col_obligatory;
	if (w==edit_flug_typ) return col_obligatory;
	if (w==edit_modus) return col_obligatory;
	if (w==edit_startart) return col_obligatory;
	if (w==edit_registration_sfz) return col_obligatory;
	return paletteBackgroundColor ();
}

