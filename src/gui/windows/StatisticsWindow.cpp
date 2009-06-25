#include "StatisticsWindow.h"

#include <QLayout>

// Columns Bordbuch
const int tbl_bob_registration=0;
const int tbl_bob_flugzeug_typ=1;
const int tbl_bob_datum=2;
const int tbl_bob_name=3;
const int tbl_bob_insassen=4;
const int tbl_bob_ort_von=5;
const int tbl_bob_ort_nach=6;
const int tbl_bob_zeit_start=7;
const int tbl_bob_zeit_landung=8;
const int tbl_bob_anzahl_landungen=9;
const int tbl_bob_betriebszeit=10;
const int tbl_bob_bemerkungen=11;
const int bob_spalten=12;

// Columns Flugbuch
const int tbl_flb_tag=0;
const int tbl_flb_muster=1;
const int tbl_flb_registration=2;
const int tbl_flb_flugzeugfuehrer=3;
const int tbl_flb_begleiter=4;
const int tbl_flb_startart=5;
const int tbl_flb_ort_start=6;
const int tbl_flb_ort_landung=7;
const int tbl_flb_zeit_start=8;
const int tbl_flb_zeit_landung=9;
const int tbl_flb_flugdauer=10;
const int tbl_flb_bemerkung=11;
const int flb_spalten=12;

// Columns Startartstatistik
const int tbl_sas_startart=0;
const int tbl_sas_anzahl=1;
const int sas_spalten=2;


StatisticsWindow::StatisticsWindow (QWidget *parent, const char *name, bool modal, WFlags f, QObject *status_dialog, sk_db *_db)
	:SkDialog (parent, name, false, f, status_dialog), db (_db)
	/*
	 * Initializes a statistics window instance.
	 * Parameters:
	 *   - parent, name, modal, f: passed to the parent class constructor.
	 *   - status_dialog: a widget accepting the status dialog interface slots.
	 *   - db: the database to use.
	 */
{
	// Create and setup table
	tab=new SkTable (this);

	// Create and setup close button
	but_close=new QPushButton (this);
	but_close->setText ("&Schlie�en");
	QObject::connect (but_close, SIGNAL (clicked ()), this, SLOT (accept ()));

	// Main layout for the table
	QVBoxLayout *layout_main=new QVBoxLayout (this, window_margin, -1, "layout_main");
	layout_main->addWidget (tab);

	// Layout for buttons under the table
	QHBoxLayout *layout_but=new QHBoxLayout (layout_main, -1, "layout_but");
	layout_but->addStretch (1);
	layout_but->addWidget (but_close);

	resize (780, 540);
}

SkTableItem *StatisticsWindow::set_table_cell (int row, int col, const QString &text, QColor bg)
	/*
	 * Sets a table item to a given text and color.
	 * Parameters:
	 *   - row, column: the coordinates of the cell to set.
	 *   - text: the text to set.
	 *   - bg: the background color to set.
	 * Return value:
	 *   - the newly created table item.
	 */
{
	// TODO: deltete the table items?
	SkTableItem *ret;
	tab->setItem (row, col, ret=new SkTableItem (text, bg));
	return ret;
}






void StatisticsWindow::fill_sastat (QDate datum)
	/*
	 * Generates and fills in the startartstatistik.
	 * Parameters:
	 *   - datum: the date to generate the statistics for.
	 */
{
	// Startarten listen
	QPtrList<LaunchType> startarten; startarten.setAutoDelete (true);
	db->list_startarten_all (startarten);
	// TODO vereinfachen
	int num_startarten=startarten.count ();
	if (num_startarten>0)
	{
		int *sa_fluege=new int[num_startarten];
		for (int i=0; i<num_startarten; i++) sa_fluege[i]=0;

		// Alle Fl�ge listen
		QPtrList<Flight> flights; flights.setAutoDelete (true);
		db->list_flights_date (flights, &datum); // TODO Fehlerbehandlung

		for (QPtrListIterator<Flight> fl (flights); *fl; ++fl)
		{
			int i=0;
			for (QPtrListIterator<LaunchType> sa (startarten); *sa; ++sa)
			{
				if ((*sa)->get_id ()==(*fl)->startart)
					sa_fluege[i]++;

				i++;
			}
		}

		// Startarten mit Anzahl ihrer Fl�ge eintragen
		int s=0;
		for (QPtrListIterator<LaunchType> sa (startarten); *sa; ++sa)
		{
			int row=s;
			tab->insertRow (s);
			set_table_cell (row, tbl_sas_startart, (*sa)->get_description ());
			set_table_cell (row, tbl_sas_anzahl, QString::number (sa_fluege[s]));
			s++;
		}

		delete[] sa_fluege;
		tab->setCurrentCell (0,0);
	}
}

void StatisticsWindow::sastat (QDate datum)
	/*
	 * Displays the window, displaying the startartstatistik.
	 * Parameters:
	 *   - datum: the date to generate statistics for.
	 */
{
	setCaption ("Startartstatistik");

	tab->setColumnCount (sas_spalten);

	tab->horizontalHeaderItem(tbl_sas_startart)->setText ("Startart");
	tab->horizontalHeaderItem(tbl_sas_anzahl)->setText ("Anzahl Starts");

	tab->setColumnWidth (tbl_sas_startart, 120);
	tab->setColumnWidth (tbl_sas_anzahl, 100);

	emit status ("Startartstatistik wird erzeugt, bitte warten...");
	emit long_operation_start ();
	qApp->processEvents ();
	fill_sastat (datum);
	emit long_operation_end ();

	show ();
}


// New code...

void StatisticsWindow::bordbuch (QDate datum)
	/*
	 * Displays the window, displaying the bordbuch.
	 * Parameters:
	 *   - datum: the date to generate statistics for.
	 */
{
	setCaption ("Bordbuch");

	// Setup table for !!Bordbuch
	tab->setColumnCount (bob_spalten);

	tab->setColumn (tbl_bob_registration, "Kennz.", 55);
	tab->setColumn (tbl_bob_flugzeug_typ, "Typ", 56);
	tab->setColumn (tbl_bob_datum, "Datum", 80);
	tab->setColumn (tbl_bob_name, "Name", 120);
	tab->setColumn (tbl_bob_insassen, "Insassen", 70);
	tab->setColumn (tbl_bob_ort_von, "Ort: Von", 140);
	tab->setColumn (tbl_bob_ort_nach, "Ort: Nach", 140);
	tab->setColumn (tbl_bob_zeit_start, "Start", 80);
	tab->setColumn (tbl_bob_zeit_landung, "Landung", 80);
	tab->setColumn (tbl_bob_anzahl_landungen, "Anz. Ldg.", 65);
	tab->setColumn (tbl_bob_betriebszeit, "Betriebsdauer", 110);
	tab->setColumn (tbl_bob_bemerkungen, "Bemerkungen", 200);

	emit status ("Bordb�cher werden erzeugt, bitte warten...");
	emit long_operation_start ();

	QPtrList<bordbuch_entry> bordbuch; bordbuch.setAutoDelete (true);
	make_bordbuch_day (bordbuch, db, datum);

	tab->hide ();
	for (QPtrListIterator<bordbuch_entry> bbe (bordbuch); *bbe; ++bbe)
	{
		// TODO emit progress
		display_bordbuch_entry (*bbe);
	}

	tab->setCurrentCell (0,0);
	tab->show ();

	emit long_operation_end ();
	show ();
}

void StatisticsWindow::flugbuch (QDate datum)
	/*
	 * Displays the window, displaying the flugbuch.
	 * Parameters:
	 *   - datum: the date to generate statistics for.
	 */
{
	setCaption ("Flugbuch");

	// Setup table for !!Flugbuch
	tab->setColumnCount (flb_spalten);

	tab->setColumn (tbl_flb_tag, "Tag", 80);
	tab->setColumn (tbl_flb_muster, "Muster", 60);
	tab->setColumn (tbl_flb_registration, "Kennz.", 56);
	tab->setColumn (tbl_flb_flugzeugfuehrer, "Pilot", 120);
	tab->setColumn (tbl_flb_begleiter, "Begleiter", 98);
	tab->setColumn (tbl_flb_startart, "Startart", 49);
	tab->setColumn (tbl_flb_ort_start, "Ort Start", 80);
	tab->setColumn (tbl_flb_ort_landung, "Ort Landung", 80);
	tab->setColumn (tbl_flb_zeit_start, "Start", 40);
	tab->setColumn (tbl_flb_zeit_landung, "Ldg.", 40);
	tab->setColumn (tbl_flb_flugdauer, "Dauer", 44);
	tab->setColumn (tbl_flb_bemerkung, "Bemerkung", 147);

	emit status ("Flugbücher werden erzeugt, bitte warten...");
	emit long_operation_start ();

	QPtrList<flugbuch_entry> flugbuch; flugbuch.setAutoDelete (true);
	make_flugbuch_day (flugbuch, db, datum);

	tab->hide ();
	for (QPtrListIterator<flugbuch_entry> fbe (flugbuch); *fbe; ++fbe)
	{
		// TODO emit progress
		display_flugbuch_entry (*fbe);
	}

	tab->setCurrentCell (0,0);
	tab->show ();

	emit long_operation_end ();
	show ();
}


void StatisticsWindow::display_bordbuch_entry (bordbuch_entry *bbe)
	/*
	 * Adds a bordbuch entry to the table.
	 * Parameters:
	 *   - bbe: the bordbuch entry to display.
	 */
{
	int row=tab->columnCount ();
	tab->insertRow (row);

	set_table_cell (row, tbl_bob_registration, bbe->registration);
	set_table_cell (row, tbl_bob_flugzeug_typ, bbe->flugzeug_typ);
	set_table_cell (row, tbl_bob_datum, bbe->datum_string ());
	set_table_cell (row, tbl_bob_name, bbe->name);
	set_table_cell (row, tbl_bob_insassen, bbe->insassen_string ());
	set_table_cell (row, tbl_bob_ort_von, bbe->ort_von);
	set_table_cell (row, tbl_bob_ort_nach, bbe->ort_nach);
	set_table_cell (row, tbl_bob_zeit_start, bbe->zeit_start_string ());
	set_table_cell (row, tbl_bob_zeit_landung, bbe->zeit_landung_string ());
	set_table_cell (row, tbl_bob_anzahl_landungen, bbe->anzahl_landungen_string ());
	set_table_cell (row, tbl_bob_betriebszeit, bbe->betriebsdauer_string ());
	set_table_cell (row, tbl_bob_bemerkungen, bbe->bemerkungen);

	if (bbe->invalid)
	{
		// The entry was found to be invalid. We mark this by addin
		// parentheses around those values of the entry which can still change.
#define add_parentheses(column) tab->item (row, column)->setText ("("+tab->item (row, column)->text ()+")");
		add_parentheses (tbl_bob_name)
		add_parentheses (tbl_bob_insassen)
		add_parentheses (tbl_bob_ort_von)
		add_parentheses (tbl_bob_ort_nach)
		add_parentheses (tbl_bob_zeit_start)
		add_parentheses (tbl_bob_zeit_landung)
		add_parentheses (tbl_bob_anzahl_landungen)
		add_parentheses (tbl_bob_betriebszeit)
#undef add_parentheses

		set_table_cell (row, tbl_bob_bemerkungen, "Eintrag ung�ltig, da noch offene Fl�ge");
	}
}

void StatisticsWindow::display_flugbuch_entry (flugbuch_entry *fbe)
	/*
	 * Adds a flugbuch entry to the table.
	 * Parameters:
	 *   - bbe: the flugbuch entry to display.
	 */
{
	int row=tab->columnCount ();
	tab->insertRow (row);
	QString bemerkung;

	set_table_cell (row, tbl_flb_tag, fbe->tag_string ());
	set_table_cell (row, tbl_flb_muster, fbe->muster);
	set_table_cell (row, tbl_flb_registration, fbe->registration);
	set_table_cell (row, tbl_flb_flugzeugfuehrer, fbe->flugzeugfuehrer);
	set_table_cell (row, tbl_flb_begleiter, fbe->begleiter);
	set_table_cell (row, tbl_flb_startart, fbe->startart);
	set_table_cell (row, tbl_flb_ort_start, fbe->ort_start);
	set_table_cell (row, tbl_flb_ort_landung, fbe->ort_landung);
	set_table_cell (row, tbl_flb_zeit_start, fbe->zeit_start_string ());
	set_table_cell (row, tbl_flb_zeit_landung, fbe->zeit_landung_string ());
	set_table_cell (row, tbl_flb_flugdauer, fbe->flugdauer_string ());
	bemerkung=fbe->bemerkung;

	if (fbe->invalid)
	{
		// The entry was found to be invalid. We mark this by addin
		// parentheses around those values of the entry which can still change.
#define add_parentheses(column) tab->item (row, column)->setText ("("+tab->item (row, column)->text ()+")");
		add_parentheses (tbl_flb_zeit_start)
		add_parentheses (tbl_flb_zeit_landung)
		add_parentheses (tbl_flb_flugdauer)
#undef add_parentheses

		if (!bemerkung.isEmpty ()) bemerkung+="; ";
		bemerkung+="Eintrag ung�ltig, da noch offene Fl�ge";
	}

	set_table_cell (row, tbl_flb_bemerkung, bemerkung);
}

