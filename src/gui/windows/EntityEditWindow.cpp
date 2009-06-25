#include "EntityEditWindow.h"

#include <QLayout>

// TODO when creating, set the focus to the first control not yet given (e. g.
// club).

//switch type template/*{{{*/
//switch (type)
//{
//	case st_none:
//		log_error ("Invalid type in EntityEditWindow:: ()");
//		break;
//	case st_plane:
//		break;
//	case st_startart:
//		break;
//	default:
//		log_error ("Unhandled type in EntityEditWindow:: ()");
//		break;
//}
///*}}}*/



EntityEditWindow::EntityEditWindow (EntityType t, QWidget *parent, sk_db *_db, const char *name, bool modal, WFlags f, QObject *status_dialog)/*{{{*/
	:SkDialog (parent, name, modal, f, status_dialog)
	/*
	 * Initializes a Entity editor window.
	 * Parameters:
	 *   - t: the type of Entity to edit.
	 *   - parent, name, modal, f: passed to the parent class constructor.
	 *   - db: the database to use.
	 *   - status_dialog: an object providing the status dialog interface.
	 */

{
	edit_registration=NULL;
	edit_wettkennz=NULL;
	edit_category=NULL;
	edit_club=NULL;
	edit_typ=NULL;
	edit_sitze=NULL;
	edit_vorname=NULL;
	edit_nachname=NULL;
	edit_bemerkungen=NULL;

	type=t;

	db=_db;

	labels=NULL;
	edit_widgets=NULL;
	num_fields=0;
	name_editable=false;

	categories=NULL; num_categories=0;

	// Widgets anlegen und einstellen/*{{{*/
	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityEditWindow::EntityEditWindow");
			break;
		case st_plane:
		{
			setCaption ("Flugzeug");

			num_fields=7;
			edit_widgets=new QWidget*[num_fields];
			edit_widgets[0]=edit_registration=new SkTextBox (this);
			edit_widgets[1]=edit_wettkennz=new SkTextBox (this);
			edit_widgets[2]=edit_category=new SkComboBox (false, this);
			edit_widgets[3]=edit_typ=new SkComboBox (true, this);
			edit_widgets[4]=edit_club=new SkComboBox (true, this);
			edit_widgets[5]=edit_sitze=new QLineEdit (this);
			edit_widgets[6]=edit_bemerkungen=new SkTextBox (this);

			edit_club->setAutoCompletion (true);
			edit_club->setInsertionPolicy (QComboBox::NoInsertion);
			edit_typ->setAutoCompletion (true);
			edit_typ->setInsertionPolicy (QComboBox::NoInsertion);
			edit_sitze->setValidator (new QIntValidator (edit_sitze));
		} break;
		case st_person:
		{
			setCaption ("Person");

			num_fields=5;
			edit_widgets=new QWidget*[num_fields];
			edit_widgets[0]=edit_nachname=new SkTextBox (this);
			edit_widgets[1]=edit_vorname=new SkTextBox (this);
			edit_widgets[2]=edit_club=new SkComboBox (true, this);
			edit_widgets[3]=edit_landesverbandsnummer=new SkTextBox (this);
			edit_widgets[4]=edit_bemerkungen=new SkTextBox (this);

			edit_club->setAutoCompletion (true);
			edit_club->setInsertionPolicy (QComboBox::NoInsertion);
		} break;
		default:
			log_error ("Unhandled type in EntityEditWindow::EntityEditWindow");
			break;
	}/*}}}*/

	edit_widgets[0]->setFocus ();

	// Weigets positionieren, Labels anlegen und positionieren/*{{{*/
	labels=new QLabel*[num_fields];
	QGridLayout *layout=new QGridLayout (this, 0, 2, widget_border, widget_gap, "layout");
	for (int i=0; i<num_fields; i++)
	{
		labels[i]=new QLabel (this);
		labels[i]->setBuddy (edit_widgets[i]);

		layout->addWidget (labels[i], i, 0);
		layout->addWidget (edit_widgets[i], i, 1);
	}/*}}}*/

	layout->setRowStretch (num_fields, 1);

	// Labels und Focus einstellen/*{{{*/
	edit_widgets[0]->setFocus ();
	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityEditWindow::EntityEditWindow");
			break;
		case st_plane:
			labels[0]->setText ("&Kennzeichen");
			labels[1]->setText ("&Wettbewerbskennzeichen");
			labels[2]->setText ("&Gattung");
			labels[3]->setText ("T&yp");
			labels[4]->setText ("&Verein");
			labels[5]->setText ("S&itze");
			labels[6]->setText ("Be&merkungen");
			break;
		case st_person:
			labels[0]->setText ("&Nachname");
			labels[1]->setText ("V&orname");
			labels[2]->setText ("&Verein");
			labels[3]->setText ("Lan&desverbandsnummer");
			labels[4]->setText ("Be&merkungen");
			break;
		default:
			log_error ("Unhandled type in EntityEditWindow::EntityEditWindow");
			break;
	}/*}}}*/

	layout->setColSpacing (1, edit_widgets[0]->width ()*2);

	but_save=new QPushButton ("&Speichern", this);
	but_cancel=new QPushButton ("&Abbrechen", this);

	but_save->setDefault (true);

	QHBoxLayout *layout_buttons=new QHBoxLayout ();
	layout_buttons->addStretch (1);
	layout_buttons->add (but_save);
	layout_buttons->add (but_cancel);

	layout->addMultiCellLayout (layout_buttons, num_fields+1, num_fields+1, 0, 1);

	QObject::connect (but_cancel, SIGNAL (clicked()), this, SLOT (reject ()));
	QObject::connect (but_save, SIGNAL (clicked()), this, SLOT (slot_save ()));

	populate_lists ();
}/*}}}*/

EntityEditWindow::~EntityEditWindow ()/*{{{*/
	/*
	 * Cleans up a Entity editor instance.
	 */
{
	if (categories) delete[] categories;
	num_categories=0;
}/*}}}*/



void EntityEditWindow::populate_lists ()/*{{{*/
	/*
	 * Fill in all lists that don't need to query the database.
	 */
{
	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityEditWindow::populte_lists ()");
			break;
		case st_plane:
		{
			// Fill in categories/*{{{*/
			edit_category->clear ();
			num_categories=list_categories (&categories, false);
			for (int i=0; i<num_categories; i++)
			{
				edit_category->insertItem (category_string (categories[i], ls_schnellzugriff), i);
			}
			edit_category->setCurrentItem (category_index (lfz_sonstige));/*}}}*/
		} break;
		case st_person:
		{
		} break;
		default:
			log_error ("Unhandled type in EntityEditWindow::populate_lists ()");
			break;
	}
}/*}}}*/

void EntityEditWindow::read_db ()/*{{{*/
	/*
	 * Reads the lists relevant to the current type from the datbase.
	 */
{
	emit long_operation_start ();

	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityEditWindow::read_db");
			break;
		case st_plane:
		{
			emit status ("Flugzeugeditor: Vereine aus Datenbank lesen...");
			// Vereine eintragen/*{{{*/
			edit_club->clear ();
			QStringList clubs;
			db->list_clubs (clubs);
			edit_club->insertStringList (clubs);
			edit_club->setEditText ("");/*}}}*/

			emit status ("Flugzeugeditor: Flugzeugtypen aus Datenbank lesen...");
			// Typen eintragen/*{{{*/
			edit_typ->clear ();
			QStringList typen;
			db->list_types (typen);
			edit_typ->insertStringList (typen);
			edit_typ->setEditText ("");/*}}}*/
		} break;
		case st_person:
		{
			emit status ("Personeneditor: Vereine aus Datenbank lesen...");
			// Vereine eintragen/*{{{*/
			edit_club->clear ();
			QStringList clubs;
			db->list_clubs (clubs);
			edit_club->insertStringList (clubs);
			edit_club->setEditText ("");/*}}}*/
		} break;
		default:
			log_error ("Unhandled type in EntityEditWindow::read_db");
			break;
	}

	emit long_operation_end ();
}/*}}}*/

void EntityEditWindow::enable_widgets (bool enable)/*{{{*/
	/*
	 * Enable or disable (make read-only) all editing widgets.
	 */
{
	for (int i=0; i<num_fields; i++) edit_widgets[i]->setEnabled (enable);
}/*}}}*/

void EntityEditWindow::setup_controls ()/*{{{*/
	/*
	 * Setup the controls, depending on the editor mode and Entity type.
	 */
{
	QString type_string;
	QString mode_string;

	switch (mode)
	{
		case em_create:/*{{{*/
		{
			mode_string="erstellen";
			enable_widgets (true);
		} break;/*}}}*/
		case em_edit:/*{{{*/
		{
			mode_string="editieren";
			enable_widgets (true);
		} break;/*}}}*/
		case em_display: /*{{{*/
		{
			mode_string="anzeigen";
			enable_widgets (false);
		} break;/*}}}*/
		case em_none:/*{{{*/
		{
			mode_string="[machen]";
			enable_widgets (true);
		} break;/*}}}*/
		default:/*{{{*/
			  log_error ("Unhandled mode in EntityEditWindow::setup_controls ()");
			  break;/*}}}*/
	}

	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityEditWindow::setup_controls ()");
			break;
		case st_plane:
			edit_registration->setEnabled (name_editable);
			edit_widgets[name_editable?0:1]->setFocus ();
			QObject::connect (edit_registration, SIGNAL (focus_out ()), this, SLOT (slot_registration ()));
//			QObject::connect (edit_registration, SIGNAL (activated (const QString &)), this, SLOT (slot_registration ()));
			type_string="Flugzeug";
			break;
		case st_person:
			if (mode==em_create) edit_nachname->setEnabled (name_editable);
			if (mode==em_create) edit_vorname->setEnabled (name_editable);
			edit_widgets[name_editable?0:2]->setFocus ();
			type_string="Person";
			break;
		default:
			log_error ("Unhandled type in EntityEditWindow::setup_controls ()");
			type_string="Zeug";
			break;
	}

	setCaption (type_string + " " + mode_string);
}/*}}}*/



void EntityEditWindow::flugzeug_eintragen (Plane *f)/*{{{*/
	/*
	 * Write a plane to the editor controls.
	 * Parameters:
	 *   - f: the plane to write.
	 */
{
	edit_registration->setText (f->registration);
	edit_wettkennz->setText (f->wettbewerbskennzeichen);
	edit_typ->setCurrentText (f->typ);
	edit_club->setCurrentText (f->club);
	edit_category->setCurrentItem (category_index (category_from_registration (f->registration)));
	edit_bemerkungen->setText (f->bemerkungen);

	if (f->sitze>0)
	{
		QString s; s.setNum (f->sitze);
		edit_sitze->setText (s);
	}
}/*}}}*/

void EntityEditWindow::person_eintragen (Person *p)/*{{{*/
	/*
	 * Write a person to the editor controls.
	 * Parameters:
	 *   - f: the person to write.
	 */
{
	edit_vorname->setText (p->vorname);
	edit_nachname->setText (p->nachname);
	edit_club->setCurrentText (p->club);
	edit_landesverbandsnummer->setText (p->landesverbands_nummer);
	edit_bemerkungen->setText (p->bemerkungen);
}/*}}}*/



void EntityEditWindow::slot_save ()/*{{{*/
	/*
	 * Accept the data and close the dialog.
	 * Called when the save button is pressed.
	 */
{
	if (accept_data ())
	{
		accept ();
	}
}/*}}}*/

void EntityEditWindow::slot_registration ()/*{{{*/
	/*
	 * Try to determine the category of the plane from the registration.
	 * Called when the focus is moved off the registration field.
	 */
{
	aircraft_category gat=category_from_registration (edit_registration->text ());
	if (gat!=lfz_keine) edit_category->setCurrentItem (category_index (gat));
}/*}}}*/



bool EntityEditWindow::check_data ()/*{{{*/
	/*
	 * Check whether the data enterede seem plausible. If not: ask the user. If
	 * the user says, data is OK, data is OK.
	 * Return value:
	 *   - true if everything is OK.
	 *   - false else.
	 */
{
	// TODO: In eine tempor�re Instanz schreiben und �berpr�fung in die Klasse schieben
	QString msg;

	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityEditWindow::check_data ()");
			break;
		case st_plane:
		{
			aircraft_category gat=categories[edit_category->currentItem ()];

			// Kennzeichen existiert schon/*{{{*/
			if (mode==em_create)
			{
				Plane plane;
				int ret=db->get_plane_registration (&plane, edit_registration->text ());
				if (ret==db_ok)
				{
					msg="Es gibt bereits ein Flugzeug mit dem Kennzeichen \""+edit_registration->text ()+"\".\n";
					QMessageBox::critical (this, "Flugzeug exisitiert bereits", msg, QMessageBox::Ok, QMessageBox::NoButton);
					return false;
				}
			}/*}}}*/

			// Kennzeichen ist leer/*{{{*/
			if (eintrag_ist_leer (edit_registration->text ()))
			{
				msg="F�r das Flugzeug wurde kein Kennzeichen angegeben.\n";
				if (!check_message (this, msg)) return false;
			}/*}}}*/

			// Typ ist leer/*{{{*/
			if (eintrag_ist_leer (edit_typ->currentText ()))
			{
				msg="F�r das Flugzeug wurde kein Typ angegeben.\n";
				if (!check_message (this, msg)) return false;
			}/*}}}*/

			// Category does not match registration/*{{{*/
			if (gat!=category_from_registration (edit_registration->text ()))
			{
				msg="Die angegebene Gattung passt nicht zum Kennzeichen.\n";
				if (!check_message (this, msg)) return false;
			}/*}}}*/

			if (gat==lfz_segelflugzeug||gat==lfz_motorsegler||gat==lfz_ultraleicht)
			{
				// Anzahl der Sitze/*{{{*/
				int sitze=edit_sitze->text ().toInt ();
				if (sitze < 1)
				{
					msg="Das Flugzeug hat etwas wenige Sitze.\n";
					if (!check_message (this, msg)) return false;
				}
				if (sitze > 2)
				{
					msg="Das Flugzeug hat etwas viele Sitze.\n";
					if (!check_message (this, msg)) return false;
				}/*}}}*/
			}
		} break;
		case st_person:
		{
		} break;
		default:
			log_error ("Unhandled type in EntityEditWindow::check_data ()");
			break;
	}

	return true;
}/*}}}*/

bool EntityEditWindow::accept_data ()/*{{{*/
	/*
	 * Accept the data, that is, check if everything is OK and, if yes, write
	 * the data to the buffer buf.
	 * Return value:
	 *   - true if the check returned OK
	 *   - false else
	 * Members writen:
	 *   - buf
	 */
{
	if (check_data ())
	{
		switch (type)
		{
			case st_none:
				log_error ("Invalid type in EntityEditWindow::accept_data ()");
				break;
			case st_plane:
			{
				Plane *plane=(Plane *)buf;

				if (name_editable)
				plane->registration=edit_registration->text ();
				plane->wettbewerbskennzeichen=edit_wettkennz->text ();
				plane->category=categories[edit_category->currentItem ()];
				plane->typ=edit_typ->currentText ();
				plane->club=edit_club->currentText ();
				plane->sitze=edit_sitze->text ().toInt ();
				plane->bemerkungen=edit_bemerkungen->text ();
			} break;
			case st_person:
			{
				Person *person=(Person *)buf;
				if (name_editable)
				{
					person->vorname=edit_vorname->text ();
					person->nachname=edit_nachname->text ();
				}
				person->club=edit_club->currentText ();
				person->landesverbands_nummer=edit_landesverbandsnummer->text ();
				person->bemerkungen=edit_bemerkungen->text ();
			} break;
			default:
				log_error ("Unhandled type in EntityEditWindow::accept_data ()");
				break;
		}

		return true;
	}
	else
	{
		return false;
	}
}/*}}}*/



void EntityEditWindow::reset ()/*{{{*/
	/*
	 * Initializes the controls.
	 */
{
	// Does nothing, fields are initialized by entering an empty Entity class.
}/*}}}*/


int EntityEditWindow::exec ()/*{{{*/
	/*
	 * Displays the dialog, after filling in the fields.
	 * Return value:
	 *   - the parent class exec return value.
	 */
{
	setup_controls ();

	switch (type)
	{
		case st_none:
			log_error ("Invalid type in EntityEditWindow::exec ()");
			break;
		case st_plane:
			flugzeug_eintragen ((Plane *)buf);
			break;
		case st_person:
			person_eintragen ((Person *)buf);
			break;
		default:
			log_error ("Unhandled type in EntityEditWindow::exec ()");
			break;
	}

	return SkDialog::exec ();
}/*}}}*/

int EntityEditWindow::create (Entity *data, bool can_change_name)/*{{{*/
	/*
	 * Displays the dialog for creating a Entity.
	 * Parameters:
	 *   - data: the buffer where the Entity is written, if accepted. It is
	 *     written to the editor before the dialog is displayed.
	 *   - can_change_name: whether the name can be edited by the user.
	 * Return value:
	 *   - the result of exec ().
	 */
{
	mode=em_create;
	if (data==NULL) return QDialog::Rejected;
	name_editable=can_change_name;
	buf=data;
	return exec ();
}/*}}}*/

int EntityEditWindow::edit (Entity *data, bool can_change_name)/*{{{*/
	/*
	 * Displays the dialog for editing a Entity.
	 * Parameters:
	 *   - data: the buffer where the Entity is written, if accepted. It is
	 *     written to the editor before the dialog is displayed.
	 *   - can_change_name: whether the name can be edited by the user.
	 * Return value:
	 *   - the result of exec ().
	 */
{
	mode=em_edit;
	if (data==NULL) return QDialog::Rejected;
	name_editable=can_change_name;
	buf=data;
	return exec ();
}/*}}}*/

int EntityEditWindow::disp (Entity *data)/*{{{*/
	/*
	 * Displays the dialog for displaying (read only) a Entity.
	 * Parameters:
	 *   - data: the buffer the Entity is read from
	 * Return value:
	 *   - the result of exec ().
	 */
{
	mode=em_display;
	if (data==NULL) return QDialog::Rejected;
	name_editable=false;
	buf=data;
	return exec ();
}/*}}}*/




int EntityEditWindow::category_index (aircraft_category gat)/*{{{*/
	/*
	 * Return the index of a given category in the category list.
	 * Parameter:
	 *   - gat: the category.
	 * Return value:
	 *   - the index of gat.
	 */
{
	for (int i=0; i<num_categories; i++)
	{
		if (categories[i]==gat) return i;
	}
	return -1;
}/*}}}*/



void EntityEditWindow::slot_db_update (db_event *event)/*{{{*/
	/*
	 * Called when the database is changed. Update lists, if applicable.
	 */
{
	// Vereine machen wir unabh�ngig vom Typ, Flugzeugtypen werden nur
	// eingetragen, wenn edit_typ existiert.
	if ((event->type==det_add||event->type==det_change) && event->id>0)
	{
		if (type==st_plane && event->table==db_flugzeug)
		{
			Plane fz;
			db->get_plane (&fz, event->id);
			if (edit_club) edit_club->insert_if_new (fz.club);
			if (edit_typ) edit_typ->insert_if_new (fz.typ);
		}
		if (type==st_person && event->table==db_person)
		{
			Person ps;
			db->get_person (&ps, event->id);
			if (edit_club) edit_club->insert_if_new (ps.club);
		}
		if (event->table==db_flug)
		{
			Flight fl;
			db->get_flight (&fl, event->id);
		}
	}

	if (event->type==det_refresh)
	{
		read_db ();
	}

	SkDialog::slot_db_update (event);
}/*}}}*/

