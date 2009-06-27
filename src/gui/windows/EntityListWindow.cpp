#include "EntityListWindow.h"

#include <QLayout>

const int id_mnu_datenbank=0;

//switch type template
//switch (type)
//{
//	case st_none:
//		log_error ("Invalid type in EntityListWindow:: ()");
//		break;
//	case st_plane:
//		break;
//	case st_startart:
//		break;
//	default:
//		log_error ("Unhandled type in EntityListWindow:: ()");
//		break;
//}
//



// Tabellenspalten Flugzeug
const int tbl_fz_registration=0;
const int tbl_fz_wettkennz=1;
const int tbl_fz_typ=2;
const int tbl_fz_category=3;
const int tbl_fz_sitze=4;
const int tbl_fz_club=5;
const int tbl_fz_bemerkungen=6;
const int tbl_fz_id=7;
const int tbl_fz_editierbar=8;
const int fz_spalten=9;

// Tabellenspalten Person
const int tbl_ps_nachname=0;
const int tbl_ps_vorname=1;
const int tbl_ps_club=2;
const int tbl_ps_lvnum=3;
const int tbl_ps_bemerkungen=4;
const int tbl_ps_id=5;
const int tbl_ps_editierbar=6;
const int ps_spalten=7;




EntityListWindow::EntityListWindow (EntityType t, QWidget *parent, Database *_db, const char *name, bool modal, WFlags f, ::SplashScreen *spl)
	:SkDialog (parent, name, modal, f), db (_db), type (t), ss (spl)
	/*
	 * Creates a Entity list instance.
	 * Parameters:
	 *   - t: the type of Entity to list.
	 *   - parent, name, model, f: passed to the base class constructor.
	 *   - _db: the database connection to use.
	 */
{
	QFrame *frame_main=new QFrame (this, "frame_main");

	but_close=new QPushButton (frame_main);
	but_close->setText ("&Schlie�en");
	QObject::connect (but_close, SIGNAL (clicked ()), this, SLOT (accept ()));

	tab=new SkTable (frame_main);
	tab->setSortingEnabled (true);

	// Men� 'Datenbank'
	menu_datenbank = new QMenu (this);
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


	QObject::connect (tab, SIGNAL (cellDoubleClicked (int, int)), this, SLOT (slot_table_double_click (int, int)));
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


	editor_fenster=new EntityEditWindow (type, this, db, "editor", true);
	QObject::connect (editor_fenster, SIGNAL (status (QString)), this, SIGNAL (status (QString)));
	db_connect (editor_fenster);

	resize (780, 540);
}

EntityListWindow::~EntityListWindow ()
	/*
	 * Cleans up a Entity list instance.
	 */
{
	// Output header widths.
//	QHeader *table_header=tab->horizontalHeader ();
//	for (int i=0; i<table_header->count (); i++)
//		printf ("%d\n", table_header->sectionSize (i));
}

SkTableItem *EntityListWindow::set_table_cell (int row, int col, const QString &text, QColor bg, db_id id)
	/*
	 * Sets a Table in the cell to a given text and background color and save an ID.
	 * Parameters:
	 *   - row, col: the row and column of the cell to set.
	 *   - text: the text to display.
	 *   - bg: the background color to set.
	 *   - id: the ID to save in the cell.
	 * Return value:
	 *   - a pointer to the newly created Table item.
	 */
{
	SkTableItem *ret=NULL;
	tab->setItem (row, col, ret=new SkTableItem (text, bg));
	ret->set_id (id);
	return ret;
}



int EntityListWindow::addEntity (EntityType t, Entity *st)
	/*
	 * Adds an entry to the list.
	 * Parameters:
	 *   - t: the type of st.
	 *   - *st: the Entity to enter.
	 * Return value:
	 *   - the newly created row number.
	 */
{
	// TODO sortiert, aber als parameter, spart zeit beim populaten
	int row=tab->rowCount();
	tab->insertRow (row);
	fillInEntity (t, row, st);
	tab->setCurrentCell (row, tab->currentColumn ());
	return row;
}

void EntityListWindow::fillInEntity (EntityType t, int row, Entity *st)
	/*
	 * Writes an entry to a given row.
	 * Parameters:
	 *   - t: the type of st
	 *   - row: the row where to write the data.
	 *   - *st: the data.
	 */
{
	(void)t;
	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityListWindow::fillInEntity ()");
			break;
		case st_plane:
		{
			Plane *fz=(Plane *)st;
			QString s; s.setNum (fz->sitze);
			set_table_cell (row, tbl_fz_registration, fz->registration, col_default, fz->id);
			set_table_cell (row, tbl_fz_wettkennz, fz->wettbewerbskennzeichen);
			set_table_cell (row, tbl_fz_typ, fz->typ);
			set_table_cell (row, tbl_fz_category, category_string (fz->category, ls_tabelle));
			set_table_cell (row, tbl_fz_sitze, s);
			set_table_cell (row, tbl_fz_club, fz->club);
			set_table_cell (row, tbl_fz_bemerkungen, fz->bemerkungen);
			set_table_cell (row, tbl_fz_id, QString::number (fz->id));
			set_table_cell (row, tbl_fz_editierbar, fz->editierbar?"Ja":"Nein");
		} break;
		case st_person:
		{
			Person *ps=(Person *)st;
			set_table_cell (row, tbl_ps_nachname, ps->nachname, col_default, ps->id);
			set_table_cell (row, tbl_ps_vorname, ps->vorname);
			set_table_cell (row, tbl_ps_club, ps->club);
			set_table_cell (row, tbl_ps_lvnum, ps->landesverbands_nummer);
			set_table_cell (row, tbl_ps_bemerkungen, ps->bemerkungen);
			set_table_cell (row, tbl_ps_id, QString::number (ps->id));
			set_table_cell (row, tbl_ps_editierbar, ps->editierbar?"Ja":"Nein");
		} break;
		default:
			log_error ("Unhandled type in EntityListWindow::fillInEntity ()");
			break;
	}
}

void EntityListWindow::listEntity (EntityType t)
	/*
	 * Lists all entries from the database to the Table.
	 * Parameters:
	 *   - t: the Entity type to list.
	 */
{
	QPtrList<Entity> list; list.setAutoDelete (true);

	setup_controls ();

	emit long_operation_start ();

	switch (t)
	{
		case st_none:
			log_error ("Invalid type in EntityListWindow::listEntity ()");
			break;
		case st_plane:
		{
			emit status ("Flugzeugliste: Flugzeuge aus Datenbank lesen...");
			QPtrList<Plane> planes; planes.setAutoDelete (false);
			// TODO error handling
			db->list_planes_all (planes);
			// MURX
			for (QPtrListIterator <Plane> plane (planes); *plane; ++plane) list.append (*plane);
		} break;
		case st_person:
		{
			emit status ("Personenliste: Personen aus Datenbank lesen...");
			QPtrList<Person> persons; persons.setAutoDelete (false);
			// TODO error handling
			db->list_persons_all (persons);
			// MURX
			for (QPtrListIterator <Person> person (persons); *person; ++person) list.append (*person);
		} break;
		default:
			log_error ("Unhandled type in ListWindow::listEntity ()");
			break;
	}
	tab->hide ();
	tab->removeAllRows ();

	// NB: 10 s f�r 500 Datens�tze, wenn einzeln in der Schleife
	int i=0, num=list.count ();
	tab->setRowCount (num);
	for (QPtrListIterator<Entity> item (list); *item; ++i, ++item)
	{
		if (i%11==0||i==num-1) emit progress (i, num-1);
		fillInEntity (t, i, *item);
	}

	tab->show ();
	tab->setCurrentCell (0, 0);

	emit long_operation_end ();
}



int EntityListWindow::deleteEntity (EntityType t, db_id id)
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
			log_error ("Invalid type in EntityListWindow::deleteEntity");
			ret=-1;
			break;
		case st_plane:
			ret=db->delete_plane (id);
			break;
		case st_person:
			ret=db->delete_person (id);
			break;
		default:
			log_error ("Unhandled type in EntityListWindow::deleteEntity");
			ret=-1;
			break;
	}

	if (ret>=0)
	{
		DbEvent e (det_delete, TableFromEntityType (t), id);
		emit db_change (&e);
	}

	return ret;
}



void EntityListWindow::setup_controls ()
	/*
	 * Sets up the controls to match the current Entity type.
	 */
{
	menu_bar->show ();

	but_close->show ();

	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityListWindow::setup_controls");
			break;
		case st_plane:
			tab->setColumnCount (fz_spalten);
			setCaption ("Flugzeuge");
			tab->showColumn (tbl_fz_editierbar);

			tab->setColumn (tbl_fz_registration, "Kennz.", 61);
			tab->setColumn (tbl_fz_wettkennz, "WK", 32);
			tab->setColumn (tbl_fz_typ, "Typ", 162);
			tab->setColumn (tbl_fz_category, "Gattung", 85);
			tab->setColumn (tbl_fz_sitze, "Sitze", 41);
			tab->setColumn (tbl_fz_club, "Verein", 200);
			tab->setColumn (tbl_fz_bemerkungen, "Bemerkungen", 200);
			tab->setColumn (tbl_fz_id, "ID", 30);
			tab->setColumn (tbl_fz_editierbar, "Editierbar", 80);
			break;
		case st_person:
			tab->setColumnCount (ps_spalten);
			setCaption ("Personen");
			tab->showColumn (tbl_ps_editierbar);

			tab->setColumnCount (ps_spalten);
			tab->setColumn (tbl_ps_nachname, "Nachname", 180);
			tab->setColumn (tbl_ps_vorname, "Vorname", 180);
			tab->setColumn (tbl_ps_club, "Verein", 200);
			tab->setColumn (tbl_ps_lvnum, "Landesverbandsnummer", 200);
			tab->setColumn (tbl_ps_bemerkungen, "Bemerkungen", 200);
			tab->setColumn (tbl_ps_id, "ID", 30);
			tab->setColumn (tbl_ps_editierbar, "Editierbar", 80);
			break;
		default:
			log_error ("Unhandled type in EntityListWindow::setup_controls");
			break;
	}

	reset ();

}

void EntityListWindow::reset ()
	/*
	 * Resets the dialog to its initial state.
	 */
{
	tab->setCurrentCell (0, 0);
	tab->setFocus ();
}


void EntityListWindow::keyPressEvent (QKeyEvent *e)
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
}


Entity *EntityListWindow::newEntity (EntityType t)
	/*
	 * Creates a new Entity instance. (MURX)
	 * Parameters:
	 *   - t: the type of Entity to create.
	 * Return value:
	 *   - a pointer to the Entity created.
	 */
{
	(void)t;
	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityListWindow::newEntity ()");
			return NULL;
			break;
		case st_plane:
			return new Plane;
			break;
		case st_person:
			return new Person;
			break;
		default:
			log_error ("Unhandled type in EntityListWindow::newEntity ()");
			return NULL;
			break;
	}
}

int EntityListWindow::entityFromId (EntityType t, Entity *b, db_id id)
	/*
	 * Retrieves a Entity from the database, given its ID.
	 * Parameters:
	 *   - t: the type of Entity.
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
			log_error ("Invalid type in EntityListWindow::entityFromId");
			break;
		case st_plane:
			return db->get_plane ((Plane *)b, id);
			break;
		case st_person:
			return db->get_person ((Person *)b, id);
			break;
		default:
			log_error ("Unhandled type in EntityListWindow::entityFromId");
			break;
	}

	return -1;
}

db_id EntityListWindow::editEntity (EntityType t, Entity *b)
	/*
	 * Writes a Entity to the database.
	 * Parameters:
	 *   - t: the type of Entity.
	 *   - b: the entry to write.
	 * Return value:
	 *   - -1 if there was a parameter error (MURX).
	 *   - the database return value else.
	 */
{
	switch (t)
	{
		case st_none:
			log_error ("Invalid type in EntityListWindow::editEntity");
			break;
		case st_plane:
			return db->write_plane ((Plane *)b);
			break;
		case st_person:
			return db->write_person ((Person *)b);
			break;
		default:
			log_error ("Unhandled type in EntityListWindow::editEntity");
			break;
	}

	return invalid_id;
}

void EntityListWindow::table_activated (int row)
	/*
	 * Called when the Table is double clicked or enter is pressed.
	 * Parameters:
	 *   - row: the row where the Table was activated.
	 */
{
	db_id id=tab->id_from_cell (row, 0);
	if (id_invalid (id)) return;

	int ret;
	Entity *p=newEntity (type);

	ret=entityFromId (type, p, id);
	if (ret==0)
	{
		if (p->editierbar)
		{
			ret=editor_fenster->edit (p, true);
			if (ret==QDialog::Accepted)
			{
				editEntity (type, p);
				DbEvent event (det_change, TableFromEntityType (type), id);
				emit db_change (&event);
			}
		}
		else
		{
			editor_fenster->disp (p);
		}
	}
	delete p;
}

void EntityListWindow::slot_table_double_click (int row, int col)
	/*
	 * Called when the Table is double clicked.
	 * Parameters:
	 *   - row, col: the row and column of the cell that was clicked.
	 *   - button: the button that was used.
	 *   - mousePos: the position of the mouse.
	 */
{
	(void)col;
	table_activated (row);
}

void EntityListWindow::slot_table_key (int key)
	/*
	 * A key was pressed on the Table.
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
}

void EntityListWindow::slot_neu ()
	/*
	 * Show the user the dialog for a new Entity and add it to the database.
	*/
{
	Entity *s=newEntity (type);

	int ret=editor_fenster->create (s, true);

	if (ret==QDialog::Accepted)
	{
		s->id=0;
		db_id id=editEntity (type, s);
		DbEvent event (det_add, TableFromEntityType (type), id);
		emit db_change (&event);
	}

	delete s;
}


bool EntityListWindow::entityUsed (EntityType type, db_id id)
	/*
	 * Determine from the database wheter the Entity has a flight.
	 * Parameters:
	 *   - type: the Entity type.
	 *   - id: the ID of the Entity in question.
	 * Return value:
	 *   - true if the Entity has a flight.
	 *   - false else.
	 */
{
	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityListWindow::entityUsed");
			break;
		case st_plane:
			return db->plane_used (id);
			break;
		case st_person:
			return db->person_used (id);
			break;
		default:
			log_error ("Unhandled type in EntityListWindow::entityUsed");
			break;
	}

	return false;
}

void EntityListWindow::tabelle_loeschen (int row)
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
		Entity *s=newEntity (type);
		if (entityFromId (type, s, id)==0)
		{
			QString bez_a=entityLabel (type, cas_akkusativ);
			QString bez_n=entityLabel (type, cas_nominativ);

			if (!s->editierbar)
			{
				show_warning (bez_n+" ist nicht editierbar.", this);
			}
			else if (entityUsed (type, id))
			{
				show_warning (bez_n+" wird verwendet und kann nicht gel�scht werden.", this);
			}
			else
			{
				int res=QMessageBox::information (this, "Wirklich l�schen?",
					bez_a+" \""+s->text_name ()+"\" wirklich l�schen?", "&Ja", "&Nein", QString::null, 0, 1);

				delete s;

				if (res==0)
				{
					res=deleteEntity (type, id);
					if (res<0)
					{
						// L�schen hat nicht geklappt
						QMessageBox::warning (this, "Fehler",
							"Fehler beim L�schen: "+db->db_error_description (res), QMessageBox::Ok, QMessageBox::NoButton);
					}
				}
			}
		}
		else
		{
			show_warning ("Fehler beim Lesen aus der Datenbank", this);
		}
	}
}

void EntityListWindow::slot_loeschen ()
	/*
	 * The "delete" menu entry was selected.
	 */
{
	tabelle_loeschen (tab->currentRow ());
}

void EntityListWindow::slot_refresh ()
	/*
	 * The "refresh" menu entry was selected. Refreshes the Table.
	 */
{
	listEntity (type);
	if (isVisible ())
	{
		// Focus geht sonst auf das Hauptfenster
		hide ();
		show ();
	}
}


void EntityListWindow::liste ()
	/*
	 * Displays the dialog.
	 */
{
	setup_controls ();
	show ();
}

void EntityListWindow::slot_db_update (DbEvent *event)
	/*
	 * The database changed. Update the Table, if appropriate.
	 * Parameters:
	 *   - event: a description of the database change.
	 */
{
	if (event->type==det_refresh) slot_refresh ();

	if (TableFromEntityType (type)==event->table)
	{
		Entity *s=newEntity (type);

		if (event->type==det_add && event->id>0)
		{
			entityFromId (type, s, event->id);
			int row=tab->row_from_column_id (event->id, 0);
			if (row<0)
			{
				addEntity (type, s);
			}
			else
			{
				// Steht schon in der Tabelle? Uh-oh, da stimmt was nicht
				log_error ("det_add in EntityListWindow::slot_db_update, aber id steht schon in der Tabelle");
				fillInEntity (type, row, s);
			}
		}

		if (event->type==det_change && event->id>0)
		{
			entityFromId (type, s, event->id);
			int row=tab->row_from_column_id (event->id, 0);
			if (row<0)
			{
				// Steht nicht in der Tabelle? Uh-oh, dann war die Tabelle unvollst�ndig.
				log_error ("det_change in EntityListWindow::slot_db_update, aber id steht nicht in der Tabelle");
				addEntity (type, s);
			}
			else
			{
				// TODO sortierung?
				fillInEntity (type, row, s);
			}
		}

		if (event->type==det_delete && event->id>0)
		{
			int row=tab->row_from_column_id (event->id, 0);
			if (row<0)
				log_error ("det_delete in EntityListWindow::slot_db_update, aber id steht nicht in der Tabelle");
			else
				tab->removeRow (row);
		}

		delete s;
	}

	SkDialog::slot_db_update (event);
}



void EntityListWindow::slot_ok ()
	/*
	 * The OK button was pressed. Close the dialog, returning the current row.
	 */
{
	done (tab->currentRow ());
}

void EntityListWindow::slot_abbrechen ()
	/*
	 * The Cancel button was pressed. Close the dialog, returning -1.
	 */
{
	done (-1);
}

void EntityListWindow::slot_editieren ()
	/*
	 * The "editieren" menu entry was selected. Edit the currently selected
	 * entry.
	 */
{
	table_activated (tab->currentRow ());
}


