#include "Person.h"

#include <iostream>

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

QString Person::pdf_name () const
	/*
	 * Returns the name of the person in a form suitable for the PDF document.
	 * Return value:
	 *   - the name.
	 */
{
	return name ();
}

QString Person::tabelle_name () const
	/*
	 * Returns the name of the person in a form suitable for the table.
	 * Return value:
	 *   - the name.
	 */
{
	if (eintrag_ist_leer (club)) return name ();
	return name ()+" ("+club+")";
}

QString Person::text_name () const
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

QString Person::bezeichnung (casus c) const
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
		case 3: return bemerkungen;
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

