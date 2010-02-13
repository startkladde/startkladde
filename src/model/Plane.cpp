#include "Plane.h"

#include <cassert>

#include "src/text.h"

Plane::Plane ():
	Entity ()
{
	initialize ();
}

Plane::Plane (db_id id):
	Entity (id)
{
	initialize ();
}

void Plane::initialize ()
{
	numSeats=0;
}

QString Plane::name () const
	/*
	 * Returns the name of the plane in a form suitable for enumerations.
	 * Return value:
	 *   - the name.
	 */
{
	return registration;
}

QString Plane::tableName () const
{
	if (eintrag_ist_leer (competitionId)) return registration;
	return QString ("%1 (%2)").arg (registration).arg (competitionId);
}

QString Plane::textName () const
	/*
	 * Returns the name of the plane in a form suitable for running text.
	 * Return value:
	 *   - the name.
	 */
{
	return name ();
}

void Plane::output (std::ostream &stream, output_format_t format)
{
	Entity::output (stream, format, false, "ID", id);
	Entity::output (stream, format, false, "Kennzeichen", registration);
	Entity::output (stream, format, false, "Wettbewerbskennzeichen", competitionId);
	Entity::output (stream, format, false, "Sitze", numSeats);
	Entity::output (stream, format, false, "Verein", club);
	Entity::output (stream, format, true, "Gattung", categoryText (category, lsLong));
}



QList<Plane::Category> Plane::listCategories (bool includeInvalid)
{
	QList<Category> result;
	result << categorySingleEngine << categoryGlider << categoryMotorglider << categoryUltralight << categoryOther;

	if (includeInvalid)
		result << categoryNone;

	return result;
}

QString Plane::categoryText (Plane::Category category, lengthSpecification lenspec)
{
	switch (lenspec)
	{
		case lsShort: case lsTable: case lsPilotLog:
			switch (category)
			{
				case categorySingleEngine: return "Einmot";
				case categoryGlider:       return "Segel";
				case categoryMotorglider:  return "MoSe";
				case categoryUltralight:   return "UL";
				case categoryOther:        return "Sonst";
				case categoryNone:         return "-";
			}
		case lsPrintout:
			switch (category)
			{
				case categorySingleEngine: return "1-mot";
				case categoryGlider:       return "Segel";
				case categoryMotorglider:  return "MoSe";
				case categoryUltralight:   return "UL";
				case categoryOther:        return "Sonst";
				case categoryNone:         return "-";
			}
		case lsLong:
			switch (category)
			{
				case categorySingleEngine: return "Motorflugzeug (einmotorig)";
				case categoryGlider:       return "Segelflugzeug";
				case categoryMotorglider:  return "Motorsegler";
				case categoryUltralight:   return "Ultraleicht";
				case categoryOther:        return "Sonstige";
				case categoryNone:         return "Keine";
			}
		case lsWithShortcut:
			switch (category)
			{
				case categorySingleEngine: return "E - Motorflugzeug (einmotorig)";
				case categoryGlider:       return "G - Segelflugzeug";
				case categoryMotorglider:  return "K - Motorsegler";
				case categoryUltralight:   return "M - Ultraleicht";
				case categoryOther:        return "S - Sonstige";
				case categoryNone:         return "- - Keine";
			}
	}

	// We must have returned by now - the compiler should catch unhandled
	// values.
	assert (false);
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

	QChar kbu = registration.at (2);

	if (kbu == '0' || kbu == '1' || kbu == '2' || kbu == '3' || kbu == '4'
		|| kbu == '5' || kbu == '6' || kbu == '7' || kbu == '8' || kbu == '9')
		return categoryGlider;
	else if (kbu.toLower () == 'e')
		return categorySingleEngine;
	else if (kbu.toLower () == 'm')
		return categoryUltralight;
	else if (kbu.toLower () == 'k')
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
		case categorySingleEngine: return -1;
		case categoryGlider: return 2;
		case categoryMotorglider: return 2;
		case categoryUltralight: return 2;
		case categoryOther: return -1;
	}

	assert (false);
	return -1;
}


bool Plane::selfLaunchOnly ()
{
	// Note that motorgliders can be glieders; and there are even some TMGs
	// which can do winch launch.
	return category==categorySingleEngine || category==categoryUltralight;
}

bool Plane::clubAwareLessThan (const Plane &p1, const Plane &p2)
{
	QString club1=simplify_club_name (p1.club);
	QString club2=simplify_club_name (p2.club);

	if (club1<club2) return true;
	if (club1>club2) return false;
	if (p1.registration<p2.registration) return true;
	if (p1.registration>p2.registration) return false;
	return false;
}

// ******************
// ** ObjectModels **
// ******************

int Plane::DefaultObjectModel::columnCount () const
{
	return 8;
}

QVariant Plane::DefaultObjectModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return "Kennzeichen";
		case 1: return "Wettbewerbskennzeichen";
		case 2: return "Typ";
		case 3: return "Gattung";
		case 4: return "Sitze";
		case 5: return "Verein";
		case 6: return "Bemerkungen";
		// TODO remove from DefaultItemModel?
		case 7: return "ID";
	}

	assert (false);
	return QVariant ();
}

QVariant Plane::DefaultObjectModel::displayData (const Plane &object, int column) const
{
	switch (column)
	{
		case 0: return object.registration;
		case 1: return object.competitionId;
		case 2: return object.type;
		case 3: return categoryText(object.category, lsTable);
		case 4: return object.numSeats>=0?QVariant (object.numSeats):QVariant ("?");
		case 5: return object.club;
		case 6: return object.comments;
		case 7: return object.id;
	}

	assert (false);
	return QVariant ();
}

QString Plane::toString () const
{
	return QString ("id=%1, registration=%2, competitionId=%3, type=%4, club=%5, category=%6, seats=%7")
		.arg (id)
		.arg (registration)
		.arg (competitionId)
		.arg (type)
		.arg (club)
		.arg (categoryText (category, lsLong))
		.arg (numSeats)
		;
}

// *******************
// ** SQL interface **
// *******************

QString Plane::dbTableName ()
{
	return "flugzeug_temp";
}

QString Plane::selectColumnList ()
{
	return "id,kennzeichen,verein,sitze,typ,gattung,wettbewerbskennzeichen,bemerkung";
}


Plane Plane::createFromQuery (const QSqlQuery &q)
{
	Plane p (q.value (0).toLongLong ());

	p.registration  =q.value (1).toString ();
	p.club          =q.value (2).toString ();
	p.numSeats      =q.value (3).toInt    ();
	p.type          =q.value (4).toString ();
	p.category      =categoryFromDb (
	                 q.value (5).toString ());
	p.competitionId =q.value (6).toString ();
	p.comments      =q.value (7).toString ();

	return p;
}

QString Plane::insertValueList ()
{
	return "(kennzeichen,verein,sitze,typ,gattung,wettbewerbskennzeichen,bemerkung) values (?,?,?,?,?,?,?)";
}

QString Plane::updateValueList ()
{
	return "kennzeichen=?, verein=?, sitze=?, typ=?, gattung=?, wettbewerbskennzeichen=?, bemerkung=?";
}

void Plane::bindValues (QSqlQuery &q) const
{
	q.addBindValue (registration);
	q.addBindValue (club);
	q.addBindValue (numSeats);
	q.addBindValue (type);
	q.addBindValue (categoryToDb (category));
	q.addBindValue (competitionId);
	q.addBindValue (comments);
}

QList<Plane> Plane::createListFromQuery (QSqlQuery &q)
{
	QList<Plane> list;

	while (q.next ())
		list.append (createFromQuery (q));

	return list;
}

// *** Enum mappers
QString Plane::categoryToDb (Category category)
{
	switch (category)
	{
		case categoryNone         : return "?";
		case categorySingleEngine : return "e";
		case categoryGlider       : return "1";
		case categoryMotorglider  : return "k";
		case categoryUltralight   : return "m";
		case categoryOther        : return "s";
		// no default
	}

	assert (false);
	return "?";
}

Plane::Category Plane::categoryFromDb (QString category)
{
	if      (category=="e") return categorySingleEngine;
	else if (category=="1") return categoryGlider;
	else if (category=="k") return categoryMotorglider;
	else if (category=="m") return categoryUltralight;
	else if (category=="s") return categoryOther;
	else                    return categoryNone;
}
