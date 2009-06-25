#include "Plane.h"

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
	 * Returns the name of the plane in a form suitable for the table.
	 * Return value:
	 *   - the name.
	 */
{
	return tabelle_name (false);
}

QString Plane::tabelle_name (bool schlepp) const
	/*
	 * Returns the name of the plane in a form suitable for the table.
	 * Parameters:
	 *   - schlepp: whether to return the name of the !!Schleppflugzeug.
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
	Entity::output (stream, format, true, "Gattung", category_string (category, ls_lang));
}


