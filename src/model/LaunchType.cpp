#include "LaunchType.h"

#include <cstdlib>

// Class management
void LaunchType::init ()
{
	id=invalid_id;
	towplane="";
	accelerator="";
	description="---";
	short_description="-";
	logbook_string="-";
	person_required=true;
	type=sat_other;
	ok=false;
}

LaunchType::LaunchType ()
	:Entity ()
{
	init ();
}

LaunchType::LaunchType (int _id, startart_type _type, QString _towplane, QString _description, QString _short_description, QString _accelerator, QString _logbook_string, bool _person_required)
	:Entity ()
{
	id=_id;
	type=_type;
	towplane=_towplane;
	description=_description;
	short_description=_short_description;
	accelerator=_accelerator;
	logbook_string=_logbook_string;
	person_required=_person_required;
}

LaunchType::LaunchType (QString desc)
	:Entity ()
{
	init ();

	QStringList split=desc.split (",");
	for (int i=0; i<split.size (); ++i)
		split[i]=split[i].simplifyWhiteSpace ();
	int n=split.count ();

	if (n>=0) id=split[0].toLongLong ();
	if (n>=1)
	{
		if (split[1].lower ()=="winch") type=sat_winch;
		else if (split[1].lower ()=="airtow") type=sat_airtow;
		else if (split[1].lower ()=="self") type=sat_self;
		else if (split[1].lower ()=="other") type=sat_other;
		else
		{
			log_error ("Unknown startart type in startart_t::startart_t (QString)");
			type=sat_other;
		}
	}
	else
	{
		type=sat_other;
	}
	if (n>=2) towplane=split[2];
	if (n>=3) description=split[3];
	if (n>=4) short_description=split[4];
	if (n>=5) accelerator=split[5];
	if (n>=6) logbook_string=split[6];
	if (n>=7 && split[7].lower ()=="false") person_required=false;
}

// Class information
QString LaunchType::name () const
{
	return description;
}

QString LaunchType::tableName () const
{
	return short_description;
}

QString LaunchType::textName () const
{
	return description;
}

QString LaunchType::getDescription (casus c) const
{
	return entityLabel (st_startart, c);
}



QString LaunchType::get_selector_value (int column_number) const
{
	switch (column_number)
	{
		case 0: return description;
		case 1: return comments;
		case 2: return QString::number (id);
		default: return QString ();
	}
}

QString LaunchType::get_selector_caption (int column_number)
{
	switch (column_number)
	{
		case 0: return "Bezeichnung";
		case 1: return "Bemerkungen";
		case 2: return "(ID)";
		default: return QString ();
	}
}



QString LaunchType::list_text () const
{
	if (accelerator.isEmpty ())
		return QString (" ")+description;
	else
		return QString (accelerator)+QString (" - ")+description;
}


QString startart_type_string (startart_type t)
{
	switch (t)
	{
		case sat_winch: return "Windenstart"; break;
		case sat_airtow: return "F-Schlepp"; break;
		case sat_self: return "Eigenstart"; break;
		case sat_other: return "Sonstige"; break;
	}

	return "???";
}

void LaunchType::output (std::ostream &stream, output_format_t format)
{
	Entity::output (stream, format, false, "ID", id);
	Entity::output (stream, format, false, "Bezeichnung", description);
	Entity::output (stream, format, true, "Typ", startart_type_string (type));
}


