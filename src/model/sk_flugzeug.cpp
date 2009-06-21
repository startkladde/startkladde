#include "sk_flugzeug.h"

#include "src/text.h"

sk_flugzeug::sk_flugzeug ()/*{{{*/
	:stuff ()
	/*
	 * Constructs an empty sk_flugzeug instance.
	 */
{
	sitze=0;
}/*}}}*/

sk_flugzeug::sk_flugzeug (string p_registration, string p_wettbewerbskennzeichen, string p_typ, string p_club, int p_sitze, db_id p_id)/*{{{*/
	/*
	 * Constructs an sk_flugzeug instance with given data.
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
}/*}}}*/

void sk_flugzeug::dump () const/*{{{*/
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
			registration.c_str(),
			wettbewerbskennzeichen.c_str(),
			typ.c_str(),
			club.c_str(),
			sitze,
			(int)id);
}/*}}}*/

string sk_flugzeug::bezeichnung (casus c) const/*{{{*/
	/*
	 * Returns a text describing the fact that this is a plane.
	 * Parameters:
	 *   - c: the grammatical case of the text.
	 * Return value:
	 *   the text.
	 */
{
	return stuff_bezeichnung (st_plane, c);
}/*}}}*/
							
string sk_flugzeug::name () const/*{{{*/
	/*
	 * Returns the name of the plane in a form suitable for enumerations.
	 * Return value:
	 *   - the name.
	 */
{
	return registration;
}/*}}}*/

string sk_flugzeug::tabelle_name () const/*{{{*/
	/*
	 * Returns the name of the plane in a form suitable for the table.
	 * Return value:
	 *   - the name.
	 */
{
	return tabelle_name (false);
}/*}}}*/

string sk_flugzeug::tabelle_name (bool schlepp) const/*{{{*/
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
}/*}}}*/

string sk_flugzeug::text_name () const/*{{{*/
	/*
	 * Returns the name of the plane in a form suitable for running text.
	 * Return value:
	 *   - the name.
	 */
{
	return name ();
}/*}}}*/

void sk_flugzeug::output (ostream &stream, output_format_t format)/*{{{*/
{
	stuff::output (stream, format, false, "ID", id);
	stuff::output (stream, format, false, "Kennzeichen", registration);
	stuff::output (stream, format, false, "Wettbewerbskennzeichen", wettbewerbskennzeichen);
	stuff::output (stream, format, false, "Sitze", sitze);
	stuff::output (stream, format, false, "Verein", club);
	stuff::output (stream, format, true, "Gattung", category_string (category, ls_lang));
}
/*}}}*/


