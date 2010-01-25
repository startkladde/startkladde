#include "Person.h"

#include <iostream>
#include <cassert>

#include "src/text.h"

Person::Person ()
	:Entity ()
	/*
	 * Constructs an empty Person instance.
	 */
{
}

Person::Person (QString vn, QString nn)
	/*
	 * Constructs an Person instance with given first and last name.
	 * Parameters:
	 *   - vn, nn: the initial values for the fields.
	 */
{
	vorname=vn;
	nachname=nn;
	id=0;
}

Person::Person (QString vn, QString nn, QString ve, QString cid, QString lvnum, db_id p_id)
	/*
	 * Constructs an Person instance with given data.
	 * Parameters:
	 *   - vn, nn, ve, vid, lvnum, id: the initial values for the fields.
	 */
{
	vorname=vn;
	nachname=nn;
	club=ve;
	club_id=cid;
	landesverbands_nummer=lvnum;
	id=p_id;
}

bool Person::operator< (const Person &o) const
{
	if (nachname<o.nachname) return true;
	if (nachname>o.nachname) return false;
	if (vorname<o.vorname) return true;
	if (vorname>o.vorname) return false;
	return false;
}

QString Person::name () const
	/*
	 * Returns the name of the person in a form suitable for enumerations.
	 * Return value:
	 *   - the name.
	 */
{
	if (eintrag_ist_leer (nachname)&&eintrag_ist_leer (vorname)) return "-";
	if (eintrag_ist_leer (nachname)) return vorname;
	if (eintrag_ist_leer (vorname)) return nachname;
	return nachname+", "+vorname;
}

QString Person::full_name () const
{
	QString l=nachname; if (l.isEmpty ()) l="?";
	QString f= vorname; if (f.isEmpty ()) f="?";

	return f+" "+l;
}

QString Person::formal_name () const
{
	QString l=nachname; if (l.isEmpty ()) l="?";
	QString f= vorname; if (f.isEmpty ()) f="?";

	return l+", "+f;
}

QString Person::pdf_name () const
	/*
	 * Returns the name of the person in a form suitable for the PDF document.
	 * Return value:
	 *   - the name.
	 */
{
	return name ();
}

QString Person::tableName () const
	/*
	 * Returns the name of the person in a form suitable for the Table.
	 * Return value:
	 *   - the name.
	 */
{
	if (eintrag_ist_leer (club)) return name ();
	return name ()+" ("+club+")";
}

QString Person::textName () const
	/*
	 * Returns the name of the person in a form suitable for running text.
	 * Return value:
	 *   - the name.
	 */
{
	if (eintrag_ist_leer (nachname)&&eintrag_ist_leer (vorname)) return "-";
	if (eintrag_ist_leer (nachname)) return vorname;
	if (eintrag_ist_leer (vorname)) return nachname;
	return vorname+" "+nachname;
}

QString Person::getDescription (casus c) const
	/*
	 * Returns a text describing the fact that this is a plane.
	 * Parameters:
	 *   - c: the grammatical case of the text.
	 * Return value:
	 *   the text.
	 */
{
	return entityLabel (st_person, c);
}

void Person::dump () const
	/*
	 * Print a description of the person to stdout. Used for debugging.
	 */
{
	std::cout << "sk_person dump: " << id << ", " << nachname << ", " << vorname << ", " << club << std::endl;
}



QString Person::get_selector_value (int column_number) const
{
	switch (column_number)
	{
		case 0: return nachname;
		case 1: return vorname;
		case 2: return club;
		case 3: return comments;
		case 4: return QString::number (id);
		default: return QString ();
	}
}

QString Person::get_selector_caption (int column_number)
{
	switch (column_number)
	{
		case 0: return "Nachname";
		case 1: return "Vorname";
		case 2: return "Verein";
		case 3: return "Bemerkungen";
		case 4: return "ID";
		default: return QString ();
	}
}

void Person::output (std::ostream &stream, output_format_t format)
{
	Entity::output (stream, format, false, "ID", id);
	Entity::output (stream, format, false, "Nachname", nachname);
	Entity::output (stream, format, false, "Vorname", vorname);
	Entity::output (stream, format, false, "Verein", club);
	Entity::output (stream, format, false, "Vereins-ID", club_id);
	Entity::output (stream, format, true, "Landesverbandsnummer", landesverbands_nummer);
}

// ******************
// ** ObjectModels **
// ******************

int Person::DefaultObjectModel::columnCount () const
{
	return 7;
}

QVariant Person::DefaultObjectModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return "Nachname";
		case 1: return "Vorname";
		case 2: return "Verein";
		case 3: return "Landesverbandsnummer";
		case 4: return "Bemerkungen";
		// TODO remove from DefaultItemModel?
		case 5: return "ID";
		case 6: return "Editierbar";
	}

	assert (false);
	return QVariant ();
}

QVariant Person::DefaultObjectModel::displayData (const Person &object, int column) const
{
	switch (column)
	{
		case 0: return object.nachname;
		case 1: return object.vorname;
		case 2: return object.club;
		case 3: return object.landesverbands_nummer;
		case 4: return object.comments;
		case 5: return object.id;
		case 6: return object.editable;
	}

	assert (false);
	return QVariant ();
}

QString Person::toString () const
{
	return QString ("id=%1, lastName=%2, firstName=%3, club=%4, clubId=%5")
		.arg (id)
		.arg (nachname)
		.arg (vorname)
		.arg (club)
		.arg (club_id)
		;
}


// *******************
// ** SQL interface **
// *******************

QString Person::dbTableName ()
{
	return "person_temp";
}

QString Person::selectColumnList ()
{
	return "id,nachname,vorname,verein,vereins_id,bemerkung";
}

Person Person::createFromQuery (const QSqlQuery &q)
{
	//	int index_firstName=query.record ().indexOf ("vorname");
	Person p (
		q.value (2).toString (),
		q.value (1).toString (),
		q.value (3).toString (),
		q.value (4).toString (),
		"",
		q.value (0).toLongLong ()
		);

	p.comments=q.value (5).toString ();

	return p;
}

QString Person::insertValueList ()
{
	return "(nachname,vorname,verein,vereins_id,bemerkung) values (?,?,?,?,?)";
}

QString Person::updateValueList ()
{
	return "nachname=?, vorname=?, verein=?, vereins_id=?, bemerkung=?";
}

void Person::bindValues (QSqlQuery &q) const
{
	q.addBindValue (nachname);
	q.addBindValue (vorname);
	q.addBindValue (club);
	q.addBindValue (club_id);
	q.addBindValue (comments);
}


QList<Person> Person::createListFromQuery (QSqlQuery &q)
{
	QList<Person> list;

	while (q.next ())
		list.append (createFromQuery (q));

	return list;
}
