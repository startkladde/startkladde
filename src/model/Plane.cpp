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
	Entity::output (stream, format, true, "Gattung", category_string (category, ls_lang));
}



int Plane::list_categories (Plane::Category **g, bool include_invalid)
	/*
	 * See top of file.
	 */
{
	Plane::Category *list=NULL;
	int num=0;

	if (include_invalid)
	{
		num=6;
		list=new Plane::Category[num];
		list[0]=categorySep;
		list[1]=categoryGlider;
		list[2]=categoryMotorglider;
		list[3]=categoryUltralight;
		list[4]=categoryOther;
		list[5]=categoryNone;
	}
	else
	{
		num=5;
		list=new Plane::Category[num];
		list[0]=categorySep;
		list[1]=categoryGlider;
		list[2]=categoryMotorglider;
		list[3]=categoryUltralight;
		list[4]=categoryOther;
	}

	*g=list;
	return num;
}

QString Plane::category_string (Plane::Category category, length_specification lenspec)
	/*
	 * See top of file.
	 */
{
	switch (lenspec)
	{
		case ls_kurz: case ls_tabelle: case ls_pilot_log:
		{
			switch (category)
			{
				case categorySep: return "Echo"; break;
				case categoryGlider: return "Segel"; break;
				case categoryMotorglider: return "MoSe"; break;
				case categoryUltralight: return "UL"; break;
				case categoryOther: return "Sonst"; break;
				case categoryNone: return "-"; break;
				default: return "?"; break;
			}
		} break;
		case ls_druck:
		{
			switch (category)
			{
				case categorySep: return "Echo"; break;
				case categoryGlider: return "Segel"; break;
				case categoryMotorglider: return "MoSe"; break;
				case categoryUltralight: return "UL"; break;
				case categoryOther: return "Sonst"; break;
				case categoryNone: return "-"; break;
				default: return "?"; break;
			}
		} break;
		case ls_lang:
		{
			switch (category)
			{
				case categorySep: return "Motorflugzeug (Echo)"; break;
				case categoryGlider: return "Segelflugzeug"; break;
				case categoryMotorglider: return "Motorsegler"; break;
				case categoryUltralight: return "Ultraleicht"; break;
				case categoryOther: return "Sonstige"; break;
				case categoryNone: return "Keine"; break;
				default: return "???"; break;
			}
		} break;
		case ls_csv:
		{
			switch (category)
			{
				case categorySep: return "Motorflugzeug (Echo)"; break;
				case categoryGlider: return "Segelflugzeug"; break;
				case categoryMotorglider: return "Motorsegler"; break;
				case categoryUltralight: return "Ultraleicht"; break;
				case categoryOther: return "Sonstige"; break;
				case categoryNone: return "Keine"; break;
				default: return "???"; break;
			}
		} break;
		case ls_schnellzugriff:
		{
			switch (category)
			{
				case categorySep: return "E - Motorflugzeug (Echo)"; break;
				case categoryGlider: return "1 - Segelflugzeug"; break;
				case categoryMotorglider: return "K - Motorsegler"; break;
				case categoryUltralight: return "M - Ultraleicht"; break;
				case categoryOther: return "Sonstige"; break;
				case categoryNone: return "- - Keine"; break;
				default: return "???"; break;
			}
		} break;
		default:
		{
			log_error ("Unbehandelte L�ngenangabe in flugtyp_string ()");
			switch (category)
			{
				case categorySep: return "[Echo]"; break;
				case categoryGlider: return "[Segel]"; break;
				case categoryMotorglider: return "[Motorsegler]"; break;
				case categoryUltralight: return "[UL]"; break;
				case categoryOther: return "[Sonst]"; break;
				case categoryNone: return "[-]"; break;
				default: return "[?]"; break;
			}
		} break;
	}

	return ("!!!");
}

Plane::Category Plane::category_from_registration (QString reg)
/*
 * Try to determine the kind of aircraft, given its registration. This is done based on german rules.
 * Parameters:
 *   - reg: the registration.
 * Return value:
 *   - the kind of aircraft, if one can be determined
 *   - categoryOther else
 */
{
	if (reg.length () < 3) return categoryNone;
	if (reg[0] != 'D') return categoryNone;
	if (reg[1] != '-') return categoryNone;

	QChar kbu = reg.at (2);

	if (kbu == '0' || kbu == '1' || kbu == '2' || kbu == '3' || kbu == '4'
		|| kbu == '5' || kbu == '6' || kbu == '7' || kbu == '8' || kbu == '9')
		return categoryGlider;
	else if (kbu.toLower () == 'e')
		return categorySep;
	else if (kbu.toLower () == 'm')
		return categoryUltralight;
	else if (kbu.toLower () == 'k')
		return categoryMotorglider;
	else
		return categoryOther;
}
