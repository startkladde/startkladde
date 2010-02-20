#include "Person.h"

#include <cassert>

#include <QSqlQuery>

#include "src/text.h"


// ******************
// ** Construction **
// ******************

Person::Person ():
	Entity ()
{
	initialize ();
}

Person::Person (db_id id):
	Entity (id)
{
	initialize ();
}

void Person::initialize ()
{
}


// ****************
// ** Comparison **
// ****************

bool Person::operator< (const Person &o) const
{
	if (nachname<o.nachname) return true;
	if (nachname>o.nachname) return false;
	if (vorname<o.vorname) return true;
	if (vorname>o.vorname) return false;
	return false;
}


// ****************
// ** Formatting **
// ****************

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

QString Person::fullName () const
{
	QString l=nachname; if (l.isEmpty ()) l="?";
	QString f= vorname; if (f.isEmpty ()) f="?";

	return f+" "+l;
}

QString Person::formalName () const
{
	QString l=nachname; if (l.isEmpty ()) l="?";
	QString f= vorname; if (f.isEmpty ()) f="?";

	return l+", "+f;
}

QString Person::formalNameWithClub () const
{
	if (eintrag_ist_leer (club)) return formalName ();
	return formalName ()+" ("+club+")";
}


// ********************************
// ** EntitySelectWindow helpers **
// ********************************

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


// *****************
// ** ObjectModel **
// *****************

int Person::DefaultObjectModel::columnCount () const
{
	return 6;
}

QVariant Person::DefaultObjectModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return "Nachname";
		case 1: return "Vorname";
		case 2: return "Verein";
		case 3: return "Bemerkungen";
		case 4: return "Vereins-ID";
		// TODO remove from DefaultItemModel?
		case 5: return "ID";
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
		case 3: return object.comments;
		case 4: return object.club_id;
		case 5: return object.id;
	}

	assert (false);
	return QVariant ();
}


// *******************
// ** SQL interface **
// *******************

QString Person::dbTableName ()
{
	return "people";
}

QString Person::selectColumnList ()
{
	return "id,last_name,first_name,club,club_id,comments";
}

Person Person::createFromQuery (const QSqlQuery &q)
{
	Person p (q.value (0).toLongLong ());

	p.nachname =q.value (1).toString ();
	p.vorname  =q.value (2).toString ();
	p.club     =q.value (3).toString ();
	p.club_id  =q.value (4).toString ();
	p.comments =q.value (5).toString ();

	return p;
}

QString Person::insertValueList ()
{
	return "(last_name,first_name,club,club_id,comments) values (?,?,?,?,?)";
}

QString Person::updateValueList ()
{
	return "last_name=?, first_name=?, club=?, club_id=?, comments=?";
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
