#include "LaunchType.h"

#include <cstdlib>
#include <cassert>

#include <QSqlQuery>
#include <QVariant>


LaunchType::LaunchType ()
	:Entity ()
{
	init ();
}

LaunchType::LaunchType (db_id id):
	Entity (id)
{
	init ();
}

// Class management
void LaunchType::init ()
{
	towplane="";
	accelerator="";
	description="---";
	short_description="-";
	logbook_string="-";
	person_required=true;
	type=sat_other;
	ok=false;
}

//LaunchType::LaunchType (int _id, startart_type _type, QString _towplane, QString _description, QString _short_description, QString _accelerator, QString _logbook_string, bool _person_required)
//	:Entity ()
//{
//	id=_id;
//	type=_type;
//	towplane=_towplane;
//	description=_description;
//	short_description=_short_description;
//	accelerator=_accelerator;
//	logbook_string=_logbook_string;
//	person_required=_person_required;
//}

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


QString LaunchType::toString () const
{
	return QString ("id=%1, description=%2 (%3), type=%4, person %5required")
		.arg (id)
		.arg (description)
		.arg (short_description)
		.arg (startart_type_string (type))
		.arg (person_required?"":"not ")
		;
}

// *******************
// ** SQL interface **
// *******************

QString LaunchType::dbTableName ()
{
	return "launch_methods";
}

QString LaunchType::selectColumnList ()
{
	return "id,name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments";
}

LaunchType LaunchType::createFromQuery (const QSqlQuery &q)
{
	LaunchType l (q.value (0).toLongLong ());

	l.description       =q.value (1).toString ();
	l.short_description =q.value (2).toString ();
	l.logbook_string    =q.value (3).toString ();
	l.accelerator       =q.value (4).toString ();
	l.type              =typeFromDb (
	                     q.value (5).toString ());
	l.towplane          =q.value (6).toString ();
	l.person_required   =q.value (7).toBool (); // FIXME ?
	l.comments          =q.value (8).toString ();

	return l;
}

QString LaunchType::insertValueList ()
{
	return "(name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments) values (?,?,?,?,?,?,?,?)";
}

QString LaunchType::updateValueList ()
{
	return "name=?, short_name=?, log_string=?, keyboard_shortcut=?, type=?, towplane_registration=?, person_required=?, comments=?";
}

void LaunchType::bindValues (QSqlQuery &q) const
{
	q.addBindValue (description);
	q.addBindValue (short_description);
	q.addBindValue (logbook_string);
	q.addBindValue (accelerator);
	q.addBindValue (typeToDb (type));
	q.addBindValue (towplane);
	q.addBindValue (person_required);
	q.addBindValue (comments);
}

QList<LaunchType> LaunchType::createListFromQuery (QSqlQuery &q)
{
	QList<LaunchType> list;
	while (q.next ()) list.append (createFromQuery (q));
	return list;
}

// *** Enum mappers
QString LaunchType::typeToDb (startart_type type)
{
	switch (type)
	{
		case sat_winch  : return "winch";
		case sat_airtow : return "airtow";
		case sat_self   : return "self";
		case sat_other  : return "other";
		// no default
	}

	assert (false);
	return "?";
}

startart_type LaunchType::typeFromDb (QString type)
{
	if      (type=="winch" ) return sat_winch;
	else if (type=="airtow") return sat_airtow;
	else if (type=="self"  ) return sat_self;
	else if (type=="other" ) return sat_other;
	else                     return sat_other;
}
