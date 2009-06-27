#include "Plane.h"

#include <cassert>

#include "src/text.h"

Plane::Plane ()
	:Entity ()
	/*
	 * Constructs an empty Plane instance.
	 */
{
	sitze=0;
}

Plane::Plane (QString p_registration, QString p_wettbewerbskennzeichen, QString p_typ, QString p_club, int p_sitze, db_id p_id)
	/*
	 * Constructs an Plane instance with given data.
	 * Parameters:
	 *   - p_registration, p_wettbewerbskennzeichen, p_typ, p_verein, p_sitze,
	 *     p_id: the initial values for the fields.
	 */
{
	// TODO remove verein
	registration=p_registration;
	wettbewerbskennzeichen=p_wettbewerbskennzeichen;
	typ=p_typ;
	club=p_club;
	sitze=p_sitze;
	id=p_id;
}

void Plane::dump () const
	/*
	 * Print a description of the plane to stdout. Used for debugging.
	 */
{
	// TODO cout
	printf("Kennzeichen %s\n"
			"Wettbewerbskennzeichen %s\n"
			"Typ %s\n"
			"Verein %s\n"
			"Sitze %d\n"
			"ID %d\n",
			registration.latin1(),
			wettbewerbskennzeichen.latin1(),
			typ.latin1(),
			club.latin1(),
			sitze,
			(int)id);
}

QString Plane::bezeichnung (casus c) const
	/*
	 * Returns a text describing the fact that this is a plane.
	 * Parameters:
	 *   - c: the grammatical case of the text.
	 * Return value:
	 *   the text.
	 */
{
	return entityLabel (st_plane, c);
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

QString Plane::tabelle_name () const
	/*
	 * Returns the name of the plane in a form suitable for the Table.
	 * Return value:
	 *   - the name.
	 */
{
	return tabelle_name (false);
}

QString Plane::tabelle_name (bool schlepp) const
	/*
	 * Returns the name of the plane in a form suitable for the Table.
	 * Parameters:
	 *   - schlepp: whether to return the name of the towplane
	 * Return value:
	 *   - the name.
	 */

{
	// TODO: wk auch bei schlepps
	if (schlepp||eintrag_ist_leer (wettbewerbskennzeichen)) return name ();
	return name ()+" ("+wettbewerbskennzeichen+")";
}

QString Plane::text_name () const
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
	Entity::output (stream, format, false, "Wettbewerbskennzeichen", wettbewerbskennzeichen);
	Entity::output (stream, format, false, "Sitze", sitze);
	Entity::output (stream, format, false, "Verein", club);
	Entity::output (stream, format, true, "Gattung", categoryString (category, lsLong));
}



QList<Plane::Category> Plane::listCategories (bool includeInvalid)
{
	QList<Category> result;
	result << categorySingleEngine << categoryGlider << categoryMotorglider << categoryUltralight << categoryOther;

	if (includeInvalid)
		result << categoryNone;

	return result;
}

QString Plane::categoryString (Plane::Category category, lengthSpecification lenspec)
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
		case lsCsv:
			switch (category)
			{
				case categorySingleEngine: return "Motorflugzeug";
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
