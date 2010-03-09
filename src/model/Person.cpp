#include "Person.h"

#include <cassert>

#include "src/text.h"
#include "src/db/result/Result.h"
#include "src/db/Query.h"


// ******************
// ** Construction **
// ******************

Person::Person ():
	Entity ()
{
	initialize ();
}

Person::Person (dbId id):
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
	if (lastName<o.lastName) return true;
	if (lastName>o.lastName) return false;
	if (firstName<o.firstName) return true;
	if (firstName>o.firstName) return false;
	return false;
}


// ****************
// ** Formatting **
// ****************

QString Person::toString () const
{
	return QString ("id=%1, lastName=%2, firstName=%3, club=%4, clubId=%5")
		.arg (id)
		.arg (lastName)
		.arg (firstName)
		.arg (club)
		.arg (clubId)
		;
}

QString Person::fullName () const
{
	QString l=lastName; if (l.isEmpty ()) l="?";
	QString f= firstName; if (f.isEmpty ()) f="?";

	return f+" "+l;
}

QString Person::formalName () const
{
	QString l=lastName; if (l.isEmpty ()) l="?";
	QString f= firstName; if (f.isEmpty ()) f="?";

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
		case 0: return lastName;
		case 1: return firstName;
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
		case 0: return object.lastName;
		case 1: return object.firstName;
		case 2: return object.club;
		case 3: return object.comments;
		case 4: return object.clubId;
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

Person Person::createFromResult (const Result &result)
{
	Person p (result.value (0).toLongLong ());

	p.lastName =result.value (1).toString ();
	p.firstName=result.value (2).toString ();
	p.club     =result.value (3).toString ();
	p.clubId   =result.value (4).toString ();
	p.comments =result.value (5).toString ();

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

void Person::bindValues (Query &q) const
{
	q.bind (lastName);
	q.bind (firstName);
	q.bind (club);
	q.bind (clubId);
	q.bind (comments);
}

QList<Person> Person::createListFromResult (Result &result)
{
	QList<Person> list;

	while (result.next ())
		list.append (createFromResult (result));

	return list;
}
