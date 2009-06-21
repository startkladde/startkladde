#include "data_types.h"

/*
 * There are different functions for all of the data types here.
 * These are:
 *
 * list_xxx (xxx **d, bool include_invalid)
 *   Makes an array of all xxxs known.
 *   Parameters:
 *     - include_invalid: also include the internal "invalid" values.
 *   Parametrs set:
 *     - *d: set to point to the array. This is newly allocated and must be freed by the caller.
 *   Return value:
 *     the size of the array allocated.
 *
 * string xxx_string (xxx d, length_specification lenspec)
 *   Generates a description for the value for a given length.
 *   Parameters:
 *     - d: the value to create the description for.
 *     - lenspec: the length (e. g. short, long, table, csv...)
 *   Return value:
 *     the description.
 */

int list_categories (aircraft_category **g, bool include_invalid)/*{{{*/
	/*
	 * See top of file.
	 */
{
	aircraft_category *list=NULL;
	int num=0;

	if (include_invalid)
	{
		num=6;
		list=new aircraft_category[num];
		list[0]=lfz_echo;
		list[1]=lfz_segelflugzeug;
		list[2]=lfz_motorsegler;
		list[3]=lfz_ultraleicht;
		list[4]=lfz_sonstige;
		list[5]=lfz_keine;
	}
	else
	{
		num=5;
		list=new aircraft_category[num];
		list[0]=lfz_echo;
		list[1]=lfz_segelflugzeug;
		list[2]=lfz_motorsegler;
		list[3]=lfz_ultraleicht;
		list[4]=lfz_sonstige;
	}

	*g=list;
	return num;
}/*}}}*/

string category_string (aircraft_category category, length_specification lenspec)/*{{{*/
	/*
	 * See top of file.
	 */
{
	switch (lenspec)
	{
		case ls_kurz: case ls_tabelle: case ls_flugbuch:/*{{{*/
		{
			switch (category)
			{
				case lfz_echo: return "Echo"; break;
				case lfz_segelflugzeug: return "Segel"; break;
				case lfz_motorsegler: return "MoSe"; break;
				case lfz_ultraleicht: return "UL"; break;
				case lfz_sonstige: return "Sonst"; break;
				case lfz_keine: return "-"; break;
				default: return "?"; break;
			}
		} break;/*}}}*/
		case ls_druck:/*{{{*/
		{
			switch (category)
			{
				case lfz_echo: return "Echo"; break;
				case lfz_segelflugzeug: return "Segel"; break;
				case lfz_motorsegler: return "MoSe"; break;
				case lfz_ultraleicht: return "UL"; break;
				case lfz_sonstige: return "Sonst"; break;
				case lfz_keine: return "-"; break;
				default: return "?"; break;
			}
		} break;/*}}}*/
		case ls_lang:/*{{{*/
		{
			switch (category)
			{
				case lfz_echo: return "Motorflugzeug (Echo)"; break;
				case lfz_segelflugzeug: return "Segelflugzeug"; break;
				case lfz_motorsegler: return "Motorsegler"; break;
				case lfz_ultraleicht: return "Ultraleicht"; break;
				case lfz_sonstige: return "Sonstige"; break;
				case lfz_keine: return "Keine"; break;
				default: return "???"; break;
			}
		} break;/*}}}*/
		case ls_csv:/*{{{*/
		{
			switch (category)
			{
				case lfz_echo: return "Motorflugzeug (Echo)"; break;
				case lfz_segelflugzeug: return "Segelflugzeug"; break;
				case lfz_motorsegler: return "Motorsegler"; break;
				case lfz_ultraleicht: return "Ultraleicht"; break;
				case lfz_sonstige: return "Sonstige"; break;
				case lfz_keine: return "Keine"; break;
				default: return "???"; break;
			}
		} break;/*}}}*/
		case ls_schnellzugriff:/*{{{*/
		{
			switch (category)
			{
				case lfz_echo: return "E - Motorflugzeug (Echo)"; break;
				case lfz_segelflugzeug: return "1 - Segelflugzeug"; break;
				case lfz_motorsegler: return "K - Motorsegler"; break;
				case lfz_ultraleicht: return "M - Ultraleicht"; break;
				case lfz_sonstige: return "Sonstige"; break;
				case lfz_keine: return "- - Keine"; break;
				default: return "???"; break;
			}
		} break;/*}}}*/
		default:/*{{{*/
		{
			log_error ("Unbehandelte L�ngenangabe in flugtyp_string ()");
			switch (category)
			{
				case lfz_echo: return "[Echo]"; break;
				case lfz_segelflugzeug: return "[Segel]"; break;
				case lfz_motorsegler: return "[Motorsegler]"; break;
				case lfz_ultraleicht: return "[UL]"; break;
				case lfz_sonstige: return "[Sonst]"; break;
				case lfz_keine: return "[-]"; break;
				default: return "[?]"; break;
			}
		} break;/*}}}*/
	}

	return ("!!!");
}/*}}}*/

aircraft_category category_from_registration (string reg)/*{{{*/
	/*
	 * Try to determine the kind of aircraft, given its registration. This is done based on german rules.
	 * Parameters:
	 *   - reg: the registration.
	 * Return value:
	 *   - the kind of aircraft, if one can be determined
	 *   - lfz_sonstige else
	 */
{
	if (reg.length ()<3) return lfz_keine;
	if (reg[0]!='D') return lfz_keine;
	if (reg[1]!='-') return lfz_keine;

	char kbu=reg[2];

	switch (kbu)
	{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			return lfz_segelflugzeug; break;
		case 'E': case 'e':
			return lfz_echo; break;
		case 'M': case 'm':
			return lfz_ultraleicht; break;
		case 'K': case 'k':
			return lfz_motorsegler; break;
		default:
			return lfz_sonstige; break;
	}
}/*}}}*/



int list_flugtyp (flug_typ **t, bool include_invalid)/*{{{*/
	/*
	 * See top of file.
	 */
{
	flug_typ *list=NULL;
	int num=0;

	if (include_invalid)
	{
		num=6;
		list=new flug_typ[num];
		list[0]=ft_kein;
		list[1]=ft_normal;
		list[2]=ft_schul_2;
		list[3]=ft_schul_1;
		list[4]=ft_gast_privat;
		list[5]=ft_gast_extern;
	}
	else
	{
		num=5;
		list=new flug_typ[num];
		list[0]=ft_normal;
		list[1]=ft_schul_2;
		list[2]=ft_schul_1;
		list[3]=ft_gast_privat;
		list[4]=ft_gast_extern;
	}

	*t=list;
	return num;
}/*}}}*/

string flugtyp_string (flug_typ typ, length_specification lenspec)/*{{{*/
	/*
	 * See top of file.
	 */
{
	switch (lenspec)
	{
		case ls_kurz:/*{{{*/
		{
			switch (typ)
			{
				case ft_kein: return "-"; break;
				case ft_normal: return "N"; break;
				case ft_schul_2: return "S2"; break;
				case ft_schul_1: return "S1"; break;
				case ft_schlepp: return "F"; break;
				case ft_gast_privat: return "G"; break;
				case ft_gast_extern: return "E"; break;
				default: return "?"; break;
			}
		} break;/*}}}*/
		case ls_druck:/*{{{*/
		{
			switch (typ)
			{
				case ft_kein: return "-"; break;
				case ft_normal: return "N"; break;
				case ft_schul_2: return "S2"; break;
				case ft_schul_1: return "S1"; break;
				case ft_schlepp: return "F"; break;
				case ft_gast_privat: return "G"; break;
				case ft_gast_extern: return "E"; break;
				default: return "?"; break;
			}
		} break;/*}}}*/
		case ls_tabelle: case ls_flugbuch:/*{{{*/
		{
			switch (typ)
			{
				case ft_kein: return "-"; break;
				case ft_normal: return "Normal"; break;
				case ft_schul_2: return "Schul (2)"; break;
				case ft_schul_1: return "Schul (1)"; break;
				case ft_schlepp: return "Schlepp"; break;
				case ft_gast_privat: return "Gast (P)"; break;
				case ft_gast_extern: return "Gast (E)"; break;
				default: return "?"; break;
			}
		} break;/*}}}*/
		case ls_lang:/*{{{*/
		{
			switch (typ)
			{
				case ft_kein: return "---"; break;
				case ft_normal: return "Normalflug"; break;
				case ft_schul_2: return "Schulungsflug (Doppelsitzig)"; break;
				case ft_schul_1: return "Schulungsflug (Einsitzig)"; break;
				case ft_schlepp: return "Schleppflug"; break;
				case ft_gast_privat: return "Gastflug (Privat)"; break;
				case ft_gast_extern: return "Gastflug (Extern)"; break;
				default: return "???"; break;
			}
		} break;/*}}}*/
		case ls_csv:/*{{{*/
		{
			switch (typ)
			{
				case ft_kein: return "---"; break;
				case ft_normal: return "Normalflug"; break;
				case ft_schul_2: return "Schulung (2)"; break;
				case ft_schul_1: return "Schulung (1)"; break;
				case ft_schlepp: return "Schlepp"; break;
				case ft_gast_privat: return "Gastflug (P)"; break;
				case ft_gast_extern: return "Gastflug (E)"; break;
				default: return "???"; break;
			}
		} break;/*}}}*/
		case ls_schnellzugriff:/*{{{*/
		{
			switch (typ)
			{
				case ft_kein: return "---"; break;
				case ft_normal: return "N - Normalflug"; break;
				case ft_schul_2: return "2 - Schulungsflug (Doppelsitzig)"; break;
				case ft_schul_1: return "1 - Schulungsflug (Einsitzig)"; break;
				case ft_schlepp: return "S - Schleppflug"; break;
				case ft_gast_privat: return "G - Gastflug (Privat)"; break;
				case ft_gast_extern: return "E - Gastflug (Extern)"; break;
				default: return "???"; break;
			}
		} break;/*}}}*/
		default:/*{{{*/
		{
			log_error ("Unbehandelte L�ngenangabe in flugtyp_string ()");
			switch (typ)
			{
				case ft_kein: return "[-]"; break;
				case ft_normal: return "[Normal]"; break;
				case ft_schul_2: return "[Schulung (2)]"; break;
				case ft_schul_1: return "[Schulung (1)]"; break;
				case ft_schlepp: return "[Schlepp]"; break;
				case ft_gast_privat: return "[Gast (P)]"; break;
				case ft_gast_extern: return "[Gast (E)]"; break;
				default: return "[?]"; break;
			}
		} break;/*}}}*/
	}

	return ("!!!");
}/*}}}*/

bool begleiter_erlaubt (flug_typ flugtyp)/*{{{*/
	/*
	 * Finds out if a copilot is allowed _to be recorded in the database_ for a
	 * given flight type.
	 * Parameters:
	 *   - flugtyp: the flight type.
	 * Return value:
	 *   - if a copilot is allowed.
	 */
{
	if (flugtyp==ft_schul_1) return false;
	if (flugtyp==ft_gast_privat) return false;		// Datenbanktechnisch gesehen jedenfalls
	if (flugtyp==ft_gast_extern) return false;		// Datenbanktechnisch gesehen jedenfalls
	return true;
}/*}}}*/



int list_modus (flug_modus **m, bool include_invalid)/*{{{*/
	/*
	 * See top of file.
	 */
{
	flug_modus *list=NULL;
	int num=0;

	if (include_invalid)
	{
		num=4;
		list=new flug_modus[num];
		list[0]=fmod_kein;
		list[1]=fmod_lokal;
		list[2]=fmod_kommt;
		list[3]=fmod_geht;
	}
	else
	{
		num=3;
		list=new flug_modus[num];
		list[0]=fmod_lokal;
		list[1]=fmod_kommt;
		list[2]=fmod_geht;
	}

	*m=list;
	return num;
}/*}}}*/

int list_sfz_modus (flug_modus **m, bool include_invalid)/*{{{*/
	/*
	 * See top of file.
	 */
{
	flug_modus *list=NULL;
	int num=0;

	if (include_invalid)
	{
		num=3;
		list=new flug_modus[num];
		list[0]=fmod_kein;
		list[1]=fmod_lokal;
		list[2]=fmod_geht;
	}
	else
	{
		num=2;
		list=new flug_modus[num];
		list[0]=fmod_lokal;
		list[1]=fmod_geht;
	}

	*m=list;
	return num;
}/*}}}*/

string modus_string (flug_modus modus, length_specification lenspec)/*{{{*/
	/*
	 * See top of file.
	 */
{
	switch (lenspec)
	{
		case ls_kurz: case ls_tabelle: case ls_druck: case ls_flugbuch:/*{{{*/
		{
			switch (modus)
			{
				case fmod_kein: return "-"; break;
				case fmod_lokal: return "Lokal"; break;
				case fmod_kommt: return "Kommt"; break;
				case fmod_geht: return "Geht"; break;
				default: return "?"; break;
			}
		} break;/*}}}*/
		case ls_lang: case ls_csv:/*{{{*/
		{
			switch (modus)
			{
				case fmod_kein: return "---"; break;
				case fmod_lokal: return "Lokal"; break;
				case fmod_kommt: return "Kommt"; break;
				case fmod_geht: return "Geht"; break;
				default: return "???"; break;
			}
		} break;/*}}}*/
		case ls_schnellzugriff:/*{{{*/
		{
			switch (modus)
			{
				case fmod_kein: return "---"; break;
				case fmod_lokal: return "Lokal"; break;
				case fmod_kommt: return "Kommt"; break;
				case fmod_geht: return "Geht"; break;
				default: return "???"; break;
			}
		} break;/*}}}*/
		default:/*{{{*/
		{
			log_error ("Unbehandelte L�ngenangabe in modus_string ()");
			switch (modus)
			{
				case fmod_kein: return "[-]"; break;
				case fmod_lokal: return "[Lokal]"; break;
				case fmod_kommt: return "[Kommt]"; break;
				case fmod_geht: return "[Geht]"; break;
				default: return "[?]"; break;
			}
		} break;/*}}}*/
	}

	return ("!!!");
}/*}}}*/



bool ist_schulung (flug_typ t)/*{{{*/
	/*
	 * Finds out if a given flight type is training.
	 * Parameters:
	 *   - t: the flight type.
	 * Return value:
	 *   - true if the type is training.
	 */
{
	return (
		t==ft_schul_2 ||
		t==ft_schul_1 ||
		false);
}
/*}}}*/

bool lands_here (flug_modus m)/*{{{*/
	/*
	 * Finds out if a flight of a given mode lands here.
	 * Parameters:
	 *   - m: the mode.
	 * Return value:
	 *   - true if the flight lands here.
	 */
{
	return (
		m==fmod_lokal ||
		m==fmod_kommt ||
		false);
}
/*}}}*/

bool starts_here (flug_modus m)/*{{{*/
	/*
	 * Finds out if a flight of a given mode starts here.
	 * Parameters:
	 *   - m: the mode.
	 * Return value:
	 *   - true if the flight lands here.
	 */
{
	return (
		m==fmod_lokal ||
		m==fmod_geht ||
		false);
}
/*}}}*/



QString std2q (string s)/*{{{*/
	/*
	 * Converts a std::string to a QString.
	 * Parameters:
	 *   - s: the source.
	 * Return value:
	 *   the converted string.
	 */
{
	if (s.empty ()) return "";
	return QString (s.c_str ());
}/*}}}*/

string q2std (QString s)/*{{{*/
	/*
	 * Converts a QString to a std::string
	 * Parameters:
	 *   - s: the source.
	 * Return value:
	 *   the converted string.
	 */
{
	if (s.isNull ()) return "";
//QT3:
//	return string ((const char *)s);
//	QT4:
	return string (s.toLatin1 ().constData ());
}/*}}}*/



db_event_table table_aus_stuff_type (stuff_type t)/*{{{*/
	/*
	 * Finds out which table to use in the db_change mechanism for a given
	 * stuff type (see StuffEditWindow).
	 * This is a workaround for the fact that there is some code duplication
	 * between these two variable types.
	 * Parameters:
	 *   - t: the stuff type.
	 * Return value:
	 *   - the table to use.
	 */
{
	switch (t)
	{
		case st_none: return db_kein; break;
		case st_plane: return db_flugzeug; break;
		case st_person: return db_person; break;
		default: return db_kein; break;
	}
}/*}}}*/


