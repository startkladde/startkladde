#include "Person.h"

#include <cassert>

#include "src/text.h"
#include "src/db/Query.h"
#include "src/db/result/Result.h"
#include "src/util/bool.h"
#include "src/util/qDate.h"
#include "src/util/qString.h"


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
	checkMedical=false;
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
	return QString ("id=%1, lastName=%2, firstName=%3, club=%4, clubId=%5, medicalValidity=%6, checkMedical=%7")
		.arg (id)
		.arg (lastName)
		.arg (firstName)
		.arg (club)
		.arg (clubId)
		.arg (medicalValidity.toString ())
		.arg (checkMedical)
		;
}

QString Person::fullName () const
{
	QString l=lastName ; if (l.isEmpty ()) l="?";
	QString f=firstName; if (f.isEmpty ()) f="?";

	return f+" "+l;
}

QString Person::formalName () const
{
	QString l=lastName ; if (l.isEmpty ()) l="?";
	QString f=firstName; if (f.isEmpty ()) f="?";

	return l+", "+f;
}

QString Person::formalNameWithClub () const
{
	if (isNone (club)) return formalName ();
	return formalName ()+" ("+club+")";
}

QString Person::getDisplayName () const
{
	return fullName ();
}


// *****************
// ** ObjectModel **
// *****************

int Person::DefaultObjectModel::columnCount () const
{
	return 8;
}

QVariant Person::DefaultObjectModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return "Nachname";
		case 1: return "Vorname";
		case 2: return "Verein";
		case 3: return "Medical bis";
		case 4: return utf8 ("Medical prüfen");
		case 5: return "Bemerkungen";
		case 6: return "Vereins-ID";
		case 7: return "ID";
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
		case 3: return object.medicalValidity.isValid ()?::toString (object.medicalValidity):QString ("Unbekannt");
		case 4: return boolToString (object.checkMedical);
		case 5: return object.comments;
		case 6: return object.clubId;
		case 7: return object.id;
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
	return "id,last_name,first_name,club,club_id,comments,medical_validity,check_medical_validity";
}

Person Person::createFromResult (const Result &result)
{
	Person p (result.value (0).toLongLong ());

	p.lastName            =result.value (1).toString ();
	p.firstName           =result.value (2).toString ();
	p.club                =result.value (3).toString ();
	p.clubId              =result.value (4).toString ();
	p.comments            =result.value (5).toString ();
	p.medicalValidity     =result.value (6).toDate ();
	p.checkMedical        =result.value (7).toBool ();

	return p;
}

QString Person::insertColumnList ()
{
	return "last_name,first_name,club,club_id,comments,medical_validity,check_medical_validity";
}

QString Person::insertPlaceholderList ()
{
	return "?,?,?,?,?,?,?";
}

void Person::bindValues (Query &q) const
{
	q.bind (lastName);
	q.bind (firstName);
	q.bind (club);
	q.bind (clubId);
	q.bind (comments);
	q.bind (medicalValidity);
	q.bind (checkMedical);
}

QList<Person> Person::createListFromResult (Result &result)
{
	QList<Person> list;

	while (result.next ())
		list.append (createFromResult (result));

	return list;
}
