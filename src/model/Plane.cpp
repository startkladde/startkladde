#include "Plane.h"

#include <cassert>

#include <QApplication>

#include "src/text.h"
#include "src/db/result/Result.h"
#include "src/db/Query.h"
#include "src/notr.h"

// ******************
// ** Construction **
// ******************

Plane::Plane ():
	Entity ()
{
	initialize ();
}

Plane::Plane (dbId id):
	Entity (id)
{
	initialize ();
}

void Plane::initialize ()
{
	numSeats=0;
}


// *********************
// ** Property access **
// *********************

bool Plane::selfLaunchOnly () const
{
	// Note that motorgliders can be gliders; and there are even some TMGs
	// which can do winch launch.
	return category==categoryAirplane || category==categoryUltralight;
}

QString Plane::fullRegistration () const
{
	if (isBlank (callsign))
		return registration;
	else if (isBlank (registration))
		return callsign;
	else
		return QString (notr ("%1 (%2)")).arg (registration, callsign);
}

QString Plane::registrationWithType () const
{
	if (isBlank (type))
		return registration;
	else if (isBlank (registration))
		return type;
	else
		return qnotr ("%1 (%2)").arg (registration, type);
}


// ****************
// ** Formatting **
// ****************

QString Plane::toString () const
{
	return qnotr ("id=%1, registration=%2, callsign=%3, type=%4, club=%5, category=%6, seats=%7")
		.arg (id)
		.arg (registration)
		.arg (callsign)
		.arg (type)
		.arg (club)
		.arg (categoryText (category))
		.arg (numSeats)
		;
}

bool Plane::clubAwareLessThan (const Plane &p1, const Plane &p2)
{
	QString club1=simplifyClubName (p1.club);
	QString club2=simplifyClubName (p2.club);

	if (club1<club2) return true;
	if (club1>club2) return false;
	if (p1.registration<p2.registration) return true;
	if (p1.registration>p2.registration) return false;
	return false;
}

QString Plane::getDisplayName () const
{
	return registration;
}


// **********************
// ** Category methods **
// **********************

QList<Plane::Category> Plane::listCategories (bool includeInvalid)
{
	QList<Category> result;
	result << categoryAirplane << categoryGlider << categoryMotorglider << categoryUltralight << categoryOther;

	if (includeInvalid)
		result << categoryNone;

	return result;
}

QString Plane::categoryText (Plane::Category category)
{
	switch (category)
	{
		case categoryAirplane:    return qApp->translate ("Plane", "Airplane");
		case categoryGlider:      return qApp->translate ("Plane", "Glider");
		case categoryMotorglider: return qApp->translate ("Plane", "Motorglider");
		case categoryUltralight:  return qApp->translate ("Plane", "Ultralight");
		case categoryOther:       return qApp->translate ("Plane", "Other");
		case categoryNone:        return qApp->translate ("Plane", "None");
		// no default
	}

	assert (!notr ("Unhandled category"));
	return notr ("?");
}

/**
 * Tries to determine the category of an aircraft from its registration. This
 * only works for countries where the category follows from the registration.
 * Currently, this is only implemented for german (D-....) registrations
 *
 * @param registration the registration
 * @return the category for the registration reg, or categoryNone or
 *         categoryOther
 */
Plane::Category Plane::categoryFromRegistration (QString registration)
{
	if (registration.length () < 3) return categoryNone;
	if (registration[0] != 'D') return categoryNone;
	if (registration[1] != '-') return categoryNone;

	QChar kbu = registration.at (2).toLower ();

	if (kbu == '0' || kbu == '1' || kbu == '2' || kbu == '3' || kbu == '4'
		|| kbu == '5' || kbu == '6' || kbu == '7' || kbu == '8' || kbu == '9'
		|| kbu == 'n')
		return categoryGlider;
	else if (kbu == 'e' || kbu == 'f' || kbu == 'g' || kbu == 'i'
		|| kbu == 'c' || kbu == 'c' || kbu == 'c')
		return categoryAirplane;
	else if (kbu == 'm')
		return categoryUltralight;
	else if (kbu == 'k')
		return categoryMotorglider;
	else
		return categoryOther;
}

/**
 * Returns the maximum number of seats in a plane of a given category, or -1
 * if there is no maximum.
 */
int Plane::categoryMaxSeats (Plane::Category category)
{
	switch (category)
	{
		case categoryNone: return -1;
		case categoryAirplane: return -1;
		case categoryGlider: return 2;
		case categoryMotorglider: return 2;
		case categoryUltralight: return 2;
		case categoryOther: return -1;
	}

	assert (false);
	return -1;
}


// *****************
// ** ObjectModel **
// *****************

int Plane::DefaultObjectModel::columnCount () const
{
	return 8;
}

QVariant Plane::DefaultObjectModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return qApp->translate ("Plane::DefaultObjectModel", "Registration");
		case 1: return qApp->translate ("Plane::DefaultObjectModel", "Callsign");
		case 2: return qApp->translate ("Plane::DefaultObjectModel", "Model");
		case 3: return qApp->translate ("Plane::DefaultObjectModel", "Category");
		case 4: return qApp->translate ("Plane::DefaultObjectModel", "Seats");
		case 5: return qApp->translate ("Plane::DefaultObjectModel", "Club");
		case 6: return qApp->translate ("Plane::DefaultObjectModel", "Comments");
		// TODO remove from DefaultItemModel?
		case 7: return qApp->translate ("Plane::DefaultObjectModel", "ID");
	}

	assert (false);
	return QVariant ();
}

QVariant Plane::DefaultObjectModel::displayData (const Plane &object, int column) const
{
	switch (column)
	{
		case 0: return object.registration;
		case 1: return object.callsign;
		case 2: return object.type;
		case 3: return categoryText(object.category);
		case 4: return object.numSeats>=0?QVariant (object.numSeats):QVariant (notr ("?"));
		case 5: return object.club;
		case 6: return object.comments;
		case 7: return object.id;
	}

	assert (false);
	return QVariant ();
}


// *******************
// ** SQL interface **
// *******************

QString Plane::dbTableName ()
{
	return notr ("planes");
}

QString Plane::selectColumnList ()
{
	return notr ("id,registration,club,num_seats,type,category,callsign,comments");
}


Plane Plane::createFromResult (const Result &result)
{
	Plane p (result.value (0).toLongLong ());

	p.registration  =result.value (1).toString ();
	p.club          =result.value (2).toString ();
	p.numSeats      =result.value (3).toInt    ();
	p.type          =result.value (4).toString ();
	p.category      =categoryFromDb (
	                 result.value (5).toString ());
	p.callsign      =result.value (6).toString ();
	p.comments      =result.value (7).toString ();

	return p;
}

QString Plane::insertColumnList ()
{
	return notr ("registration,club,num_seats,type,category,callsign,comments");
}

QString Plane::insertPlaceholderList ()
{
	return notr ("?,?,?,?,?,?,?");
}

void Plane::bindValues (Query &q) const
{
	q.bind (registration);
	q.bind (club);
	q.bind (numSeats);
	q.bind (type);
	q.bind (categoryToDb (category));
	q.bind (callsign);
	q.bind (comments);
}

QList<Plane> Plane::createListFromResult (Result &result)
{
	QList<Plane> list;

	while (result.next ())
		list.append (createFromResult (result));

	return list;
}

// *** Enum mappers
QString Plane::categoryToDb (Category category)
{
	switch (category)
	{
		case categoryNone         : return notr ("?")          ;
		case categoryAirplane     : return notr ("airplane")   ;
		case categoryGlider       : return notr ("glider")     ;
		case categoryMotorglider  : return notr ("motorglider");
		case categoryUltralight   : return notr ("ultralight") ;
		case categoryOther        : return notr ("other")      ;
		// no default
	}

	assert (!notr ("Unhandled category"));
	return notr ("?");
}

Plane::Category Plane::categoryFromDb (QString category)
{
	if      (category==notr ("airplane")   ) return categoryAirplane;
	else if (category==notr ("glider")     ) return categoryGlider;
	else if (category==notr ("motorglider")) return categoryMotorglider;
	else if (category==notr ("ultralight") ) return categoryUltralight;
	else if (category==notr ("other")      ) return categoryOther;
	else                              return categoryNone;
}
