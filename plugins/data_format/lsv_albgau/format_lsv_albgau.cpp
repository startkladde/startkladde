#include <string>

#include <src/object_field.h>
#include <src/model/sk_flug.h>
#include <src/model/sk_flugzeug.h>
#include <src/model/sk_person.h>
#include <src/model/startart_t.h>
#include <src/plugins/plugin_data_format.h>
#include <src/web/argument.h>

const string format_lsv_albgau="lsv_albgau";

extern "C" argument_list list_formats ()/*{{{*/
{
	argument_list l;
	l.set_value (format_lsv_albgau, "LSV Albgau");
	return l;
}
/*}}}*/

// _FIELDS_
const string field_name_datum="datum";
const string field_name_startzeit_stunden="startzeit_stunden";
const string field_name_startzeit_minuten="startzeit_minuten";
const string field_name_landezeit_stunden="landezeit_stunden";
const string field_name_landezeit_minuten="landezeit_minuten";
const string field_name_flugzeug_kennzeichen="flugzeug_kennzeichen";
const string field_name_flugzeug_verein="flugzeug_verein";
const string field_name_flugart="flugart";
const string field_name_startart="startart";
const string field_name_pilot_nachname="pilot_nachname";
const string field_name_pilot_vorname="pilot_vorname";
const string field_name_pilot_code="pilot_code";
const string field_name_pilot_verein="pilot_verein";
const string field_name_begleiter_nachname="begleiter_nachname";
const string field_name_begleiter_vorname="begleiter_vorname";
const string field_name_begleiter_code="begleiter_code";
const string field_name_bemerkungen="bemerkung";
const string field_name_abrechnungshinweis="abrechnungshinweis";
const string field_name_id="record_id";

extern "C" void make_field_list (const string &format, list<object_field> &fields)
{
	if (format==format_lsv_albgau)
	{
		// _FIELDS_
		fields.push_back (object_field::output_field ("Datum", field_name_datum));
		fields.push_back (object_field::output_field ("Startzeit Stunden", field_name_startzeit_stunden));
		fields.push_back (object_field::output_field ("Startzeit Minuten", field_name_startzeit_minuten));
		fields.push_back (object_field::output_field ("Landezeit Stunden", field_name_landezeit_stunden));
		fields.push_back (object_field::output_field ("Landezeit Minuten", field_name_landezeit_minuten));
		fields.push_back (object_field::output_field ("Flugzeug Kennzeichen", field_name_flugzeug_kennzeichen));
		fields.push_back (object_field::output_field ("Flugzeug Verein", field_name_flugzeug_verein));
		fields.push_back (object_field::output_field ("Flugart", field_name_flugart));
		fields.push_back (object_field::output_field ("Startart", field_name_startart));
		fields.push_back (object_field::output_field ("Pilot Nachname", field_name_pilot_nachname));
		fields.push_back (object_field::output_field ("Pilot Vorname", field_name_pilot_vorname));
		fields.push_back (object_field::output_field ("Pilot Code", field_name_pilot_code));
		fields.push_back (object_field::output_field ("Pilot Verein", field_name_pilot_verein));
		fields.push_back (object_field::output_field ("Begleiter Nachname", field_name_begleiter_nachname));
		fields.push_back (object_field::output_field ("Begleiter Vorname", field_name_begleiter_vorname));
		fields.push_back (object_field::output_field ("Begleiter Code", field_name_begleiter_code));
		fields.push_back (object_field::output_field ("Bemerkungen", field_name_bemerkungen));
		fields.push_back (object_field::output_field ("Abrechnungshinweis", field_name_abrechnungshinweis));
		fields.push_back (object_field::output_field ("ID", field_name_id));
	}
	else
	{
		throw plugin_data_format::ex_plugin_invalid_format (format);
	}
}

extern "C" void flight_to_fields (const string &format, list<object_field> &fields, const sk_flug &f, const sk_flug_data &flight_data, int &num, const string &none_text, const string &error_text)
{
	if (format!=format_lsv_albgau) throw plugin_data_format::ex_plugin_invalid_format (format);

	//bool is_airtow=flight_data.towplane.given;

	// Iterate over all fields and, depending on the label, read the data from
	// the flight and write it to the field.
	// Depending on the list used, not all of these fields are set.
	list<object_field>::const_iterator end=fields.end ();
	// TODO: all the values are determined from the beginning for each field.
	// Move the field value determination before this loop (wonder where i've
	// been when I coded this...).
	for (list<object_field>::iterator field=fields.begin (); field!=end; ++field)
	{
		// Make the display text for the flight_data items:
		//   - If the item is not given, use the text for "none"
		//   - If the item is given but not OK, use the text for "error"
		//   - If the item is OK, use the real text.
#define ITEM(NAME, FLIGHT_DATA_MEMBER, VALUE);	\
	string NAME;	\
	if (!flight_data.FLIGHT_DATA_MEMBER.given) NAME=none_text;	\
	else if (!flight_data.FLIGHT_DATA_MEMBER.ok) NAME=error_text;	\
	else NAME=flight_data.FLIGHT_DATA_MEMBER.data->VALUE;

	ITEM (registration,          plane,    registration            )
	ITEM (plane_club,            plane,    club                    )
#undef ITEM


//	// Make the display text for items read directly from the flight.
#define CONDITIONAL(CONDITION, VALUE) ((CONDITION)?(VALUE):none_text)
		string date         =CONDITIONAL (f.happened (),                          q2std (f.effdatum ().toString ("yyyyMMdd")));
		string start_hour   =CONDITIONAL (f.flight_starts_here () && f.gestartet, f.startzeit.to_string ("%H", tz_utc, 3, true));
		string start_minute =CONDITIONAL (f.flight_starts_here () && f.gestartet, f.startzeit.to_string ("%M", tz_utc, 3, true));
		string land_hour    =CONDITIONAL (f.flight_lands_here () && f.gelandet,   f.landezeit.to_string ("%H", tz_utc, 3, true));
		string land_minute  =CONDITIONAL (f.flight_lands_here () && f.gelandet,   f.landezeit.to_string ("%M", tz_utc, 3, true));
#undef CONDITION

		// Flugart (siehe unter Flugart)
		string flight_type=error_text;
		QString accounting_note=(std2q (f.abrechnungshinweis));

// Qt3
//#define CASE_INSENSITIVE false
// Qt4
#define CASE_INSENSITIVE Qt::CaseInsensitive
		if (accounting_note.contains ("Bezahlt", CASE_INSENSITIVE))
			flight_type="B";
		else if (accounting_note.contains ("Werkstattflug", CASE_INSENSITIVE))
			flight_type="W";
		else if (accounting_note.contains ("Pauschal", CASE_INSENSITIVE))
			flight_type="P";
		else if (accounting_note.contains ("Kinderfliegen", CASE_INSENSITIVE))
			flight_type="K";
		else
		{
			switch (f.flugtyp)
			{
				case ft_gast_extern: flight_type="�"; break;
				case ft_gast_privat: flight_type="�"; break;
				case ft_kein: flight_type=none_text; break;
				case ft_schlepp: flight_type="-"; break;
				case ft_schul_1: flight_type="E"; break;
				case ft_schul_2: flight_type="S"; break;
				case ft_normal: flight_type="�"; break;
			}
		}

		// Startart (WE WA WK FS SS SO)
		string startart=error_text;
		if (f.flight_starts_here () && flight_data.startart.given)
		{
			if (flight_data.startart.ok)
			{
				startart_type sat=flight_data.startart.data->get_type ();

				switch (sat)
				{
					case sat_airtow: startart="FS"; break;
					case sat_self: startart="SS"; break;
					case sat_winch: startart=flight_data.startart.data->get_csv_string (); break;
					case sat_other: startart="SO"; break;
				}
			}
			else
			{
				startart=error_text;
			}
		}
		else
		{
			startart=none_text;
		}

		// Pilot and copilot
		string org_pilot_last_name, org_pilot_first_name, org_pilot_club, org_pilot_club_id;
		string org_copilot_last_name, org_copilot_first_name, org_copilot_club, org_copilot_club_id;

		// TODO 3 of these 4 cases are handled by ITEM
#define HANDLE_PERSON(FLIGHT_DATA_MEMBER, FIRST_NAME_VAR, LAST_NAME_VAR, CLUB_VAR, CLUB_ID_VAR, PARTIAL_FIRST_NAME, PARTIAL_LAST_NAME)	\
		do	\
		{	\
			if (flight_data.FLIGHT_DATA_MEMBER.given)	\
			{	\
				if (flight_data.FLIGHT_DATA_MEMBER.ok)	\
				{	\
					/* Given and OK ==> use it */	\
					FIRST_NAME_VAR=flight_data.FLIGHT_DATA_MEMBER.data->vorname;	\
					LAST_NAME_VAR=flight_data.FLIGHT_DATA_MEMBER.data->nachname;	\
					CLUB_VAR=flight_data.FLIGHT_DATA_MEMBER.data->club;	\
					CLUB_ID_VAR=flight_data.FLIGHT_DATA_MEMBER.data->club_id;	\
				}	\
				else	\
				{	\
					/* Given, but not OK ==> Error */	\
					FIRST_NAME_VAR=error_text;	\
					LAST_NAME_VAR=error_text;	\
					CLUB_VAR=error_text;	\
					CLUB_ID_VAR=error_text;	\
				}	\
			}	\
			else	\
			{	\
				if (PARTIAL_FIRST_NAME.empty () && PARTIAL_LAST_NAME.empty ())	/* Note the condition mixing first and last name */	\
				{	\
					/* Not given at all ==> None */	\
					FIRST_NAME_VAR=none_text;	\
					LAST_NAME_VAR=none_text;	\
					CLUB_VAR=none_text;	\
					CLUB_ID_VAR=none_text;	\
				}	\
				else	\
				{	\
					/* Partially given ==> use partial information */	\
					FIRST_NAME_VAR=PARTIAL_FIRST_NAME;	\
					LAST_NAME_VAR=PARTIAL_LAST_NAME;	\
					/* Must be error_text rather than none_text because \
					 * otherwise the case of an	unknown person (partial first \
					 * name and partial last name given) could not be \
					 * distinguished from the valid case of a person without a \
					 * club and a club ID. \
					 */	\
					CLUB_VAR=error_text;	\
					CLUB_ID_VAR=error_text;	\
				}	\
			}	\
		} while (false)

		HANDLE_PERSON (pilot, org_pilot_first_name, org_pilot_last_name, org_pilot_club, org_pilot_club_id, f.pvn, f.pnn);
		HANDLE_PERSON (copilot, org_copilot_first_name, org_copilot_last_name, org_copilot_club, org_copilot_club_id, f.bvn, f.bnn);
#undef HANDLE_PERSON

		string pilot_last_name, pilot_first_name, pilot_code, pilot_club;
		string copilot_last_name, copilot_first_name, copilot_code;

		if (ist_schulung (f.flugtyp))
		{
			// Kinda funny specification: For flight training, names are
			// swapped, but club and code aren't.
			// The flight instructor is the copilot in the database.

			// Lehrer Nachname
			pilot_last_name=org_copilot_last_name;
			// Lehrer Vorname
			pilot_first_name=org_copilot_first_name;
			// Sch�ler Code
			pilot_club=org_pilot_club_id;
			// Sch�ler Verein
			pilot_code=org_pilot_club;
			// Sch�ler Nachname
			copilot_last_name=org_pilot_last_name;
			// Sch�ler Vorname
			copilot_first_name=org_pilot_first_name;
			// Lehrer Code
			copilot_code=org_copilot_club_id;
		}
		else
		{

			// Pilot Nachname
			pilot_last_name=org_pilot_last_name;
			// Pilot Vornamen
			pilot_first_name=org_pilot_first_name;
			// Pilot Code
			pilot_club=org_pilot_club_id;
			// Pilot Verein
			pilot_code=org_pilot_club;
			// Begleiter Nachname
			copilot_last_name=org_copilot_last_name;
			// Begleiter Vorname
			copilot_first_name=org_copilot_first_name;
			// Begleiter Code
			copilot_code=org_copilot_club_id;
		}

		string label=(*field).get_label ();

		// _FIELDS_
		if (label==field_name_datum) (*field).set_to (date);
		else if (label==field_name_startzeit_stunden) (*field).set_to (start_hour);
		else if (label==field_name_startzeit_minuten) (*field).set_to (start_minute);
		else if (label==field_name_landezeit_stunden) (*field).set_to (land_hour);
		else if (label==field_name_landezeit_minuten) (*field).set_to (land_minute);
		else if (label==field_name_flugzeug_kennzeichen) (*field).set_to (registration);
		else if (label==field_name_flugzeug_verein) (*field).set_to (plane_club);
		else if (label==field_name_flugart) (*field).set_to (flight_type);
		else if (label==field_name_startart) (*field).set_to (startart);
		else if (label==field_name_pilot_nachname) (*field).set_to (pilot_last_name);
		else if (label==field_name_pilot_vorname) (*field).set_to (pilot_first_name);
		else if (label==field_name_pilot_code) (*field).set_to (pilot_code);
		else if (label==field_name_pilot_verein) (*field).set_to (pilot_club);
		else if (label==field_name_begleiter_nachname) (*field).set_to (copilot_last_name);
		else if (label==field_name_begleiter_vorname) (*field).set_to (copilot_first_name);
		else if (label==field_name_begleiter_code) (*field).set_to (copilot_code);
		else if (label==field_name_bemerkungen) (*field).set_to (f.bemerkungen);
		else if (label==field_name_abrechnungshinweis) (*field).set_to (f.abrechnungshinweis);
		else if (label==field_name_id) (*field).set_to (num_to_string (f.id));
		else
			throw plugin_data_format::ex_plugin_internal_error ("Unbehandeltes Feld \""+(*field).get_caption ()+"\" in flight_to_fields");
	}
}

