#include "StuffListWindow.h"

#include "qlayout.h"

const int id_mnu_datenbank=0;

//switch type template/*{{{*/
//switch (type)
//{
//	case st_none:
//		log_error ("Invalid type in StuffListWindow:: ()");
//		break;
//	case st_plane:
//		break;
//	case st_startart:
//		break;
//	default:
//		log_error ("Unhandled type in StuffListWindow:: ()");
//		break;
//}
///*}}}*/



// Tabellenspalten Flugzeug/*{{{*/
const int tbl_fz_registration=0;
const int tbl_fz_wettkennz=1;
const int tbl_fz_typ=2;
const int tbl_fz_category=3;
const int tbl_fz_sitze=4;
const int tbl_fz_club=5;
const int tbl_fz_bemerkungen=6;
const int tbl_fz_id=7;
const int tbl_fz_editierbar=8;
const int fz_spalten=9;/*}}}*/

// Tabellenspalten Person/*{{{*/
const int tbl_ps_nachname=0;
const int tbl_ps_vorname=1;
const int tbl_ps_club=2;
const int tbl_ps_lvnum=3;
const int tbl_ps_bemerkungen=4;
const int tbl_ps_id=5;
const int tbl_ps_editierbar=6;
const int ps_spalten=7;/*}}}*/




StuffListWindow::StuffListWindow (stuff_type t, QWidget *parent, sk_db *_db, const char *name, bool modal, WFlags f, ::SplashScreen *spl)/*{{{*/
	:SkDialog (parent, name, modal, f), db (_db), type (t), ss (spl)
	/*
	 * Creates a stuff list instance.
	 * Parameters:
	 *   - t: the type of stuff to list.
	 *   - parent, name, model, f: passed to the base class constructor.
	 *   - _db: the database connection to use.
	 */
{
	QFrame *frame_main=new QFrame (this, "frame_main");

	but_close=new QPushButton (frame_main);
	but_close->setText ("&Schlie�en");
	QObject::connect (but_close, SIGNAL (clicked ()), this, SLOT (accept ()));

	tab=new SkTable (frame_main);
	tab->setSorting (true);

	// Men� 'Datenbank'
	menu_datenbank = new QPopupMenu (this, "menu_datenbank");
	menu_datenbank->insertTearOffHandle ();
	menu_datenbank->insertItem ("&Neu", this, SLOT (slot_neu ()), CTRL+Key_N);
	menu_datenbank->insertItem ("&L�schen", this, SLOT (slot_loeschen ()), CTRL+Key_L);
	menu_datenbank->insertItem ("&Editieren", this, SLOT (slot_editieren ()), CTRL+Key_E);
	menu_datenbank->insertSeparator ();
	menu_datenbank->insertItem ("Liste aktualisie&ren", this, SLOT (slot_refresh ()), CTRL+Key_R);
	menu_datenbank->insertSeparator ();
	menu_datenbank->insertItem ("&Schlie�en", this, SLOT (accept ()), CTRL+Key_W);

	// Men�leiste
	menu_bar = new QMenuBar (this, "menu_bar");
	menu_bar->insertItem ("&Datenbank", menu_datenbank, id_mnu_datenbank);


	QObject::connect (tab, SIGNAL (doubleClicked (int, int, int, const QPoint&)), this, SLOT (slot_table_double_click (int, int, int, const QPoint&)));
	QObject::connect (tab, SIGNAL (key (int)), this, SLOT (slot_table_key (int)));
//	TODO Kontextmen�
//	QObject::connect (tab, SIGNAL (contextMenuRequested(int,int,const QPoint&)), this, SLOT(slot_table_context (int,int,const QPoint &)));

	setup_controls ();

	// Murx to combine menu with layout. Probably, a QDialog should not have a
	// menu bar.
	QVBoxLayout *layout_menu=new QVBoxLayout (this, 0, 0, "layout_menu");
	layout_menu->addWidget (menu_bar);
	layout_menu->addWidget (frame_main);

	QVBoxLayout *layout_main=new QVBoxLayout (frame_main, window_margin, -1, "layout_main");
	layout_main->addWidget (tab);

	QHBoxLayout *layout_but=new QHBoxLayout (layout_main, -1, "layout_but");
	layout_but->addStretch (1);
	layout_but->addWidget (but_close);


	editor_fenster=new StuffEditWindow (type, this, db, "editor", true);
	QObject::connect (editor_fenster, SIGNAL (status (QString)), this, SIGNAL (status (QString)));
	db_connect (editor_fenster);

	resize (780, 540);
}/*}}}*/

StuffListWindow::~StuffListWindow ()/*{{{*/
	/*
	 * Cleans up a stuff list instance.
	 */
{
	// Output header widths.
//	QHeader *table_header=tab->horizontalHeader ();
//	for (int i=0; i<table_header->count (); i++)
//		printf ("%d\n", table_header->sectionSize (i));
}/*}}}*/

sk_table_item *StuffListWindow::set_table_cell (int row, int col, const string &text, QColor bg, db_id id)/*{{{*/
	/*
	 * Sets a table in the cell to a given text and background color and save an ID.
	 * Parameters:
	 *   - row, col: the row and column of the cell to set.
	 *   - text: the text to display.
	 *   - bg: the background color to set.
	 *   - id: the ID to save in the cell.
	 * Return value:
	 *   - a pointer to the newly created table item.
	 */
{
	sk_table_item *ret=NULL;
	tab->setItem (row, col, ret=new sk_table_item (tab, text, bg));
	ret->set_id (id);
	return ret;
}/*}}}*/



int StuffListWindow::add_stuff (stuff_type t, stuff *st)/*{{{*/
	/*
	 * Adds an entry to the list.
	 * Parameters:
	 *   - t: the type of st.
	 *   - *st: the stuff to enter.
	 * Return value:
	 *   - the newly created row number.
	 */
{
	// TODO sortiert, aber als parameter, spart zeit beim populaten
	int row=tab->numRows ();
	tab->insertRows (row);
	stuff_eintragen (t, row, st);
	tab->setCurrentCell (row, tab->currentColumn ());
	return row;
}/*}}}*/

void StuffListWindow::stuff_eintragen (stuff_type t, int row, stuff *st)/*{{{*/
	/*
	 * Writes an entry to a given row.
	 * Parameters:
	 *   - t: the type of st
	 *   - row: the row where to write the data.
	 *   - *st: the data.
	 */
{
	switch (type)
	{
		case st_none:
			log_error ("Invalid type in sk_win_stuff_list::stuff_eintragen ()");
			break;
		case st_plane:
		{
			sk_flugzeug *fz=(sk_flugzeug *)st;
			QString s; s.setNum (fz->sitze);
			set_table_cell (row, tbl_fz_registration, fz->registration, col_default, fz->id);
			set_table_cell (row, tbl_fz_wettkennz, fz->wettbewerbskennzeichen);
			set_table_cell (row, tbl_fz_typ, fz->typ);
			set_table_cell (row, tbl_fz_category, category_string (fz->category, ls_tabelle));
			set_table_cell (row, tbl_fz_sitze, q2std (s));
			set_table_cell (row, tbl_fz_club, fz->club);
			set_table_cell (row, tbl_fz_bemerkungen, fz->bemerkungen);
			set_table_cell (row, tbl_fz_id, num_to_string (fz->id));
			set_table_cell (row, tbl_fz_editierbar, fz->editierbar?"Ja":"Nein");
		} break;
		case st_person:
		{
			sk_person *ps=(sk_person *)st;
			set_table_cell (row, tbl_ps_nachname, ps->nachname, col_default, ps->id);
			set_table_cell (row, tbl_ps_vorname, ps->vorname);
			set_table_cell (row, tbl_ps_club, ps->club);
			set_table_cell (row, tbl_ps_lvnum, ps->landesverbands_nummer);
			set_table_cell (row, tbl_ps_bemerkungen, ps->bemerkungen);
			set_table_cell (row, tbl_ps_id, num_to_string (ps->id));
			set_table_cell (row, tbl_ps_editierbar, ps->editierbar?"Ja":"Nein");
		} break;
		default:
			log_error ("Unhandled type in sk_win_stuff_list::stuff_eintragen ()");
			break;
	}
}/*}}}*/

void StuffListWindow::list_stuff (stuff_type t)/*{{{*/
	/*
	 * Lists all entries from the database to the table.
	 * Parameters:
	 *   - t: the stuff type to list.
	 */
{
	QPtrList<stuff> list; list.setAutoDelete (true);

	setup_controls ();

	emit long_operation_start ();

	switch (t)
	{
		case st_none:
			log_error ("Invalid type in sk_win_stuff_list:: ()");
			break;
		case st_plane:
		{
			emit status ("Flugzeugliste: Flugzeuge aus Datenbank lesen...");
			QPtrList<sk_flugzeug> planes; planes.setAutoDelete (false);
			// TODO error handling
			db->list_planes_all (planes);
			// MURX
			for (QPtrListIterator <sk_flugzeug> plane (planes); *plane; ++plane) list.append (*plane);
		} break;
		case st_person:
		{
			emit status ("Personenliste: Personen aus Datenbank lesen...");
			QPtrList<sk_person> persons; persons.setAutoDelete (false);
			// TODO error handling
			db->list_persons_all (persons);
			// MURX
			for (QPtrListIterator <sk_person> person (persons); *person; ++person) list.append (*person);
		} break;
		default:
			log_error ("Unhandled type in sk_win_stuff_list:: ()");
			break;
	}
	tab->hide ();
	tab->clear_table ();

	// NB: 10 s f�r 500 Datens�tze, wenn einzeln in der Schleife
	int i=0, num=list.count ();
	tab->insertRows (0, num);
	for (QPtrListIterator<stuff> item (list); *item; ++i, ++item)
	{
		if (i%11==0||i==num-1) emit progress (i, num-1);
		stuff_eintragen (t, i, *item);
	}

	tab->show ();
	tab->setCurrentCell (0, 0);

	emit long_operation_end ();
}/*}}}*/



int StuffListWindow::stuff_loeschen (stuff_type t, db_id id)/*{{{*/
	/*
	 * Deletes an entry from the database.
	 * Paremters:
	 *   - t: the type of the entry to delete.
	 *   - id: the entry to delete.
	 * Return value:
	 *   - the result of the database operation, or -1 if there was a parameter
	 *     error (MURX).
	 */
{
	int ret=-1;

	switch (t)
	{
		case st_none:
			log_error ("Invalid type in sk_win_stuff_list:: ()");
			ret=-1;
			break;
		case st_plane:
			ret=db->delete_plane (id);
			break;
		case st_person:
			ret=db->delete_person (id);
			break;
		default:
			log_error ("Unhandled type in sk_win_stuff_list:: ()");
			ret=-1;
			break;
	}

	if (ret>=0)
	{
		db_event e (det_delete, table_aus_stuff_type (t), id);
		emit db_change (&e);
	}

	return ret;
}/*}}}*/



void StuffListWindow::setup_controls ()/*{{{*/
	/*
	 * Sets up the controls to match the current stuff type.
	 */
{
	QHeader *table_header=tab->horizontalHeader ();

	menu_bar->show ();

	but_close->show ();

	switch (type)
	{
		case st_none:
			log_error ("Invalid type in sk_win_stuff_list::setup_controls ()");
			break;
		case st_plane:
			tab->setNumCols (fz_spalten);
			setCaption ("Flugzeuge");
			tab->showColumn (tbl_fz_editierbar);

			table_header->setLabel (tbl_fz_registration, "Kennz.", 61);
			table_header->setLabel (tbl_fz_wettkennz, "WK", 32);
			table_header->setLabel (tbl_fz_typ, "Typ", 162);
			table_header->setLabel (tbl_fz_category, "Gattung", 85);
			table_header->setLabel (tbl_fz_sitze, "Sitze", 41);
			table_header->setLabel (tbl_fz_club, "Verein", 200);
			table_header->setLabel (tbl_fz_bemerkungen, "Bemerkungen", 200);
			table_header->setLabel (tbl_fz_id, "ID", 30);
			table_header->setLabel (tbl_fz_editierbar, "Editierbar", 80);
			break;
		case st_person:
			tab->setNumCols (ps_spalten);
			setCaption ("Personen");
			tab->showColumn (tbl_ps_editierbar);

			tab->setNumCols (ps_spalten);
			table_header->setLabel (tbl_ps_nachname, "Nachname", 180);
			table_header->setLabel (tbl_ps_vorname, "Vorname", 180);
			table_header->setLabel (tbl_ps_club, "Verein", 200);
			table_header->setLabel (tbl_ps_lvnum, "Landesverbandsnummer", 200);
			table_header->setLabel (tbl_ps_bemerkungen, "Bemerkungen", 200);
			table_header->setLabel (tbl_ps_id, "ID", 30);
			table_header->setLabel (tbl_ps_editierbar, "Editierbar", 80);
			break;
		default:
			log_error ("Unhandled type in sk_win_stuff_list::setup_controls ()");
			break;
	}

	reset ();

}/*}}}*/

void StuffListWindow::reset ()/*{{{*/
	/*
	 * Resets the dialog to its initial state.
	 */
{
	tab->setCurrentCell (0, 0);
	tab->setFocus ();
}/*}}}*/


void StuffListWindow::keyPressEvent (QKeyEvent *e)/*{{{*/
	/*
	 * Handles key shortcuts.
	 * Parameters:
	 *   - e: the key event passed by the QT library.
	 */
{
	switch (e->key ())
	{
		case Qt::Key_F2: slot_neu (); break;
		case Qt::Key_F4: slot_editieren (); break;
		case Qt::Key_F8: slot_loeschen (); break;
		case Qt::Key_F12: slot_refresh (); break;
		case Qt::Key_Escape:
			accept ();
			// Hier nicht ::kPE aufrufen. e->accept () scheint nicht zu
			// funktionieren.
			break;
		case Qt::Key_Return:
			// MURX: Bei Return spricht sonst der "Schlie�en"-Button an
			e->accept ();
			break;
		default:
			e->ignore ();
			SkDialog::keyPressEvent (e);
			break;
	}
}/*}}}*/


stuff *StuffListWindow::stuff_new (stuff_type t)/*{{{*/
	/*
	 * Creates a new stuff instance. (MURX)
	 * Parameters:
	 *   - t: the type of stuff to create.
	 * Return value:
	 *   - a pointer to the stuff created.
	 */
{
	switch (type)
	{
		case st_none:
			log_error ("Invalid type in sk_win_stuff_list::stuff_new ()");
			return NULL;
			break;
		case st_plane:
			return new sk_flugzeug;
			break;
		case st_person:
			return new sk_person;
			break;
		default:
			log_error ("Unhandled type in sk_win_stuff_list::stuff_new ()");
			return NULL;
			break;
	}
}/*}}}*/

int StuffListWindow::stuff_aus_id (stuff_type t, stuff *b, db_id id)/*{{{*/
	/*
	 * Retrieves a stuff from the database, given its ID.
	 * Parameters:
	 *   - t: the type of stuff.
	 *   - b: the buffer where the result is written.
	 *   - id: the ID of the element to retrieve.
	 * Return value:
	 *   - -1 if there was a parameter error (MURX).
	 *   - the database return value else.
	 */
{
	switch (t)
	{
		case st_none:
			log_error ("Invalid type in sk_win_stuff_list::stuff_aus_id ()");
			break;
		case st_plane:
			return db->get_plane ((sk_flugzeug *)b, id);
			break;
		case st_person:
			return db->get_person ((sk_person *)b, id);
			break;
		default:
			log_error ("Unhandled type in sk_win_stuff_list::stuff_aus_id ()");
			break;
	}

	return -1;
}/*}}}*/

db_id StuffListWindow::stuff_editieren (stuff_type t, stuff *b)/*{{{*/
	/*
	 * Writes a stuff to the database.
	 * Parameters:
	 *   - t: the type of stuff.
	 *   - b: the entry to write.
	 * Return value:
	 *   - -1 if there was a parameter error (MURX).
	 *   - the database return value else.
	 */
{
	switch (t)
	{
		case st_none:
			log_error ("Invalid type in sk_win_stuff_list::stuff_editieren ()");
			break;
		case st_plane:
			return db->write_plane ((sk_flugzeug *)b);
			break;
		case st_person:
			return db->write_person ((sk_person *)b);
			break;
		default:
			log_error ("Unhandled type in sk_win_stuff_list::stuff_editieren ()");
			break;
	}

	return invalid_id;
}/*}}}*/

void StuffListWindow::table_activated (int row)/*{{{*/
	/*
	 * Called when the table is double clicked or enter is pressed.
	 * Parameters:
	 *   - row: the row where the table was activated.
	 */
{
	db_id id=tab->id_from_cell (row, 0);
	if (id_invalid (id)) return;

	int ret;
	stuff *p=stuff_new (type);

	ret=stuff_aus_id (type, p, id);
	if (ret==0)
	{
		if (p->editierbar)
		{
			ret=editor_fenster->edit (p, true);
			if (ret==QDialog::Accepted)
			{
				stuff_editieren (type, p);
				db_event event (det_change, table_aus_stuff_type (type), id);
				emit db_change (&event);
			}
		}
		else
		{
			editor_fenster->disp (p);
		}
	}
	delete p;
}/*}}}*/

void StuffListWindow::slot_table_double_click (int row, int col, int button, const QPoint &mousePos)/*{{{*/
	/*
	 * Called when the table is double clicked.
	 * Parameters:
	 *   - row, col: the row and column of the cell that was clicked.
	 *   - button: the button that was used.
	 *   - mousePos: the position of the mouse.
	 */
{
	table_activated (row);
}/*}}}*/

void StuffListWindow::slot_table_key (int key)/*{{{*/
	/*
	 * A key was pressed on the table.
	 * Parameters:
	 *   - key: the key, as determined by the QT library.
	 */
{
	if (key==Qt::Key_Return)
	{
		int orow=tab->currentRow ();
		int ocol=tab->currentColumn ();

		table_activated (tab->currentRow ());

		tab->setCurrentCell (orow, ocol);
	}

	if (key==Qt::Key_Delete) tabelle_loeschen (tab->currentRow ());
	if (key==Qt::Key_Insert) slot_neu ();
}/*}}}*/

void StuffListWindow::slot_neu ()/*{{{*/
	/*
	 * Show the user the dialog for a new stuff and add it to the database.
	*/
{
	stuff *s=stuff_new (type);

	int ret=editor_fenster->create (s, true);

	if (ret==QDialog::Accepted)
	{
		s->id=0;
		db_id id=stuff_editieren (type, s);
		db_event event (det_add, table_aus_stuff_type (type), id);
		emit db_change (&event);
	}

	delete s;
}/*}}}*/


bool StuffListWindow::stuff_verwendet (stuff_type type, db_id id)/*{{{*/
	/*
	 * Determine from the database wheter the stuff has a flight.
	 * Parameters:
	 *   - type: the stuff type.
	 *   - id: the ID of the stuff in question.
	 * Return value:
	 *   - true if the stuff has a flight.
	 *   - false else.
	 */
{
	switch (type)
	{
		case st_none:
			log_error ("Invalid type in sk_win_stuff_list:: ()");
			break;
		case st_plane:
			return db->plane_used (id);
			break;
		case st_person:
			return db->person_used (id);
			break;
		default:
			log_error ("Unhandled type in sk_win_stuff_list:: ()");
			break;
	}

	return false;
}
/*}}}*/

void StuffListWindow::tabelle_loeschen (int row)/*{{{*/
	/*
	 * Removes an entry from the database after checking if that is possible
	 * and confirming with the user.
	 * Parameters:
	 *   - row: the row of the entry to delete.
	 */
{
	db_id id=tab->id_from_cell (row, 0);
	if (id>0)
	{
		stuff *s=stuff_new (type);
		if (stuff_aus_id (type, s, id)==0)
		{
			QString bez_a=std2q (stuff_bezeichnung (type, cas_akkusativ));
			QString bez_n=std2q (stuff_bezeichnung (type, cas_nominativ));

			if (!s->editierbar)
			{
				show_warning (bez_n+" ist nicht editierbar.", this);
			}
			else if (stuff_verwendet (type, id))
			{
				show_warning (bez_n+" wird verwendet und kann nicht gel�scht werden.", this);
			}
			else
			{
				int res=QMessageBox::information (this, "Wirklich l�schen?",
					bez_a+" \""+std2q (s->text_name ())+"\" wirklich l�schen?", "&Ja", "&Nein", QString::null, 0, 1);

				delete s;

				if (res==0)
				{
					res=stuff_loeschen (type, id);
					if (res<0)
					{
						// L�schen hat nicht geklappt
						QMessageBox::warning (this, "Fehler",
							"Fehler beim L�schen: "+std2q (db->db_error_description (res)), QMessageBox::Ok, QMessageBox::NoButton);
					}
				}
			}
		}
		else
		{
			show_warning ("Fehler beim Lesen aus der Datenbank", this);
		}
	}
}/*}}}*/

void StuffListWindow::slot_loeschen ()/*{{{*/
	/*
	 * The "delete" menu entry was selected.
	 */
{
	tabelle_loeschen (tab->currentRow ());
}/*}}}*/

void StuffListWindow::slot_refresh ()/*{{{*/
	/*
	 * The "refresh" menu entry was selected. Refreshes the table.
	 */
{
	list_stuff (type);
	if (isVisible ())
	{
		// Focus geht sonst auf das Hauptfenster
		hide ();
		show ();
	}
}/*}}}*/


void StuffListWindow::liste ()/*{{{*/
	/*
	 * Displays the dialog.
	 */
{
	setup_controls ();
	show ();
}/*}}}*/

void StuffListWindow::slot_db_update (db_event *event)/*{{{*/
	/*
	 * The database changed. Update the table, if appropriate.
	 * Parameters:
	 *   - event: a description of the database change.
	 */
{
	if (event->type==det_refresh) slot_refresh ();

	if (table_aus_stuff_type (type)==event->table)
	{
		stuff *s=stuff_new (type);

		if (event->type==det_add && event->id>0)/*{{{*/
		{
			stuff_aus_id (type, s, event->id);
			int row=tab->row_from_column_id (event->id, 0);
			if (row<0)
			{
				add_stuff (type, s);
			}
			else
			{
				// Steht schon in der Tabelle? Uh-oh, da stimmt was nicht
				log_error ("det_add in sk_win_stuff_list::slot_db_update (), aber id steht schon in der Tabelle");
				stuff_eintragen (type, row, s);
			}
		}/*}}}*/

		if (event->type==det_change && event->id>0)/*{{{*/
		{
			stuff_aus_id (type, s, event->id);
			int row=tab->row_from_column_id (event->id, 0);
			if (row<0)
			{
				// Steht nicht in der Tabelle? Uh-oh, dann war die Tabelle unvollst�ndig.
				log_error ("det_change in sk_win_stuff_list::slot_db_update (), aber id steht nicht in der Tabelle");
				add_stuff (type, s);
			}
			else
			{
				// TODO sortierung?
				stuff_eintragen (type, row, s);
			}
		}/*}}}*/

		if (event->type==det_delete && event->id>0)/*{{{*/
		{
			int row=tab->row_from_column_id (event->id, 0);
			if (row<0)
				log_error ("det_delete in sk_win_stuff_list::slot_db_update (), aber id steht nicht in der Tabelle");
			else
				tab->removeRow (row);
		}/*}}}*/

		delete s;
	}

	SkDialog::slot_db_update (event);
}/*}}}*/



void StuffListWindow::slot_ok ()/*{{{*/
	/*
	 * The OK button was pressed. Close the dialog, returning the current row.
	 */
{
	done (tab->currentRow ());
}/*}}}*/

void StuffListWindow::slot_abbrechen ()/*{{{*/
	/*
	 * The Cancel button was pressed. Close the dialog, returning -1.
	 */
{
	done (-1);
}/*}}}*/

void StuffListWindow::slot_editieren ()/*{{{*/
	/*
	 * The "editieren" menu entry was selected. Edit the currently selected
	 * entry.
	 */
{
	table_activated (tab->currentRow ());
}
/*}}}*/


