#include "sk_person.h"
#include <iostream>
#include "text.h"

sk_person::sk_person ()/*{{{*/
	:stuff ()
	/*
	 * Constructs an empty sk_person instance.
	 */
{
}/*}}}*/

sk_person::sk_person (string vn, string nn)/*{{{*/
	/*
	 * Constructs an sk_person instance with given first and last name.
	 * Parameters:
	 *   - vn, nn: the initial values for the fields.
	 */
{
	vorname=vn;
	nachname=nn;
	id=0;
}/*}}}*/

sk_person::sk_person (string vn, string nn, string ve, string cid, string lvnum, db_id p_id)/*{{{*/
	/*
	 * Constructs an sk_person instance with given data.
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
}/*}}}*/

string sk_person::name () const/*{{{*/
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
}/*}}}*/

string sk_person::pdf_name () const/*{{{*/
	/*
	 * Returns the name of the person in a form suitable for the PDF document.
	 * Return value:
	 *   - the name.
	 */
{
	return name ();
}
/*}}}*/

string sk_person::tabelle_name () const/*{{{*/
	/*
	 * Returns the name of the person in a form suitable for the table.
	 * Return value:
	 *   - the name.
	 */
{
	if (eintrag_ist_leer (club)) return name ();
	return name ()+" ("+club+")";
}/*}}}*/

string sk_person::text_name () const/*{{{*/
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
}/*}}}*/

string sk_person::bezeichnung (casus c) const/*{{{*/
	/*
	 * Returns a text describing the fact that this is a plane.
	 * Parameters:
	 *   - c: the grammatical case of the text.
	 * Return value:
	 *   the text.
	 */
{
	return stuff_bezeichnung (st_person, c);
}/*}}}*/

void sk_person::dump () const/*{{{*/
	/*
	 * Print a description of the person to stdout. Used for debugging.
	 */
{
	cout << "sk_person dump: " << id << ", " << nachname << ", " << vorname << ", " << club << endl;
}/*}}}*/



string sk_person::get_selector_value (int column_number) const/*{{{*/
{
	switch (column_number)
	{
		case 0: return nachname;
		case 1: return vorname;
		case 2: return club;
		case 3: return bemerkungen;
		case 4: return num_to_string (id);
		default: return string ();
	}
}
/*}}}*/

string sk_person::get_selector_caption (int column_number)/*{{{*/
{
	switch (column_number)
	{
		case 0: return "Nachname";
		case 1: return "Vorname";
		case 2: return "Verein";
		case 3: return "Bemerkungen";
		case 4: return "ID";
		default: return string ();
	}
}
/*}}}*/

void sk_person::output (ostream &stream, output_format_t format)/*{{{*/
{
	stuff::output (stream, format, false, "ID", id);
	stuff::output (stream, format, false, "Nachname", nachname);
	stuff::output (stream, format, false, "Vorname", vorname);
	stuff::output (stream, format, false, "Verein", club);
	stuff::output (stream, format, false, "Vereins-ID", club_id);
	stuff::output (stream, format, true, "Landesverbandsnummer", landesverbands_nummer);
}
/*}}}*/

