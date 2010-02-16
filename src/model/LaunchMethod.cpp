#include "LaunchMethod.h"

#include <cstdlib>
#include <cassert>

#include <QSqlQuery>
#include <QVariant>


LaunchMethod::LaunchMethod ()
	:Entity ()
{
	init ();
}

LaunchMethod::LaunchMethod (db_id id):
	Entity (id)
{
	init ();
}

// Class management
void LaunchMethod::init ()
{
	towplaneRegistration="";
	keyboardShortcut="";
	name="---";
	shortName="-";
	logString="-";
	personRequired=true;
	type=typeOther;
}

LaunchMethod LaunchMethod::parseConfigLine (QString line)
{
	LaunchMethod l;

	QStringList split=line.split (",");
	for (int i=0; i<split.size (); ++i)
		split[i]=split[i].simplifyWhiteSpace ();
	int n=split.count ();

	if (n>=0) l.id=split[0].toLongLong ();
	if (n>=1)
	{
		if      (split[1].lower ()=="winch" ) l.type=typeWinch;
		else if (split[1].lower ()=="airtow") l.type=typeAirtow;
		else if (split[1].lower ()=="self"  ) l.type=typeSelf;
		else if (split[1].lower ()=="other" ) l.type=typeOther;
		else
		{
			log_error ("Unknown launch method type in LaunchMethod::parseConfigLine (QString)");
			l.type=typeOther;
		}
	}
	else
	{
		l.type=typeOther;
	}
	if (n>=2) l.towplaneRegistration=split[2];
	if (n>=3) l.name=split[3];
	if (n>=4) l.shortName=split[4];
	if (n>=5) l.keyboardShortcut=split[5];
	if (n>=6) l.logString=split[6];
	if (n>=7 && split[7].lower ()=="false") l.personRequired=false;

	return l;
}

// Class information
QString LaunchMethod::getName () const
{
	return name;
}

QString LaunchMethod::getTableName () const
{
	return shortName;
}

QString LaunchMethod::getTextName () const
{
	return name;
}

QString LaunchMethod::get_selector_value (int column_number) const
{
	switch (column_number)
	{
		case 0: return name;
		case 1: return comments;
		case 2: return QString::number (id);
		default: return QString ();
	}
}

QString LaunchMethod::get_selector_caption (int column_number)
{
	switch (column_number)
	{
		case 0: return "Bezeichnung";
		case 1: return "Bemerkungen";
		case 2: return "(ID)";
		default: return QString ();
	}
}



QString LaunchMethod::list_text () const
{
	if (keyboardShortcut.isEmpty ())
		return QString (" ")+name;
	else
		return QString (keyboardShortcut)+QString (" - ")+name;
}


QString LaunchMethod::typeString (LaunchMethod::Type type)
{
	switch (type)
	{
		case typeWinch: return "Windenstart"; break;
		case typeAirtow: return "F-Schlepp"; break;
		case typeSelf: return "Eigenstart"; break;
		case typeOther: return "Sonstige"; break;
	}

	return "???";
}

void LaunchMethod::output (std::ostream &stream, output_format_t format)
{
	Entity::output (stream, format, false, "ID", id);
	Entity::output (stream, format, false, "Bezeichnung", name);
	Entity::output (stream, format, true, "Typ", typeString (type));
}


QString LaunchMethod::toString () const
{
	return QString ("id=%1, description=%2 (%3), type=%4, person %5required")
		.arg (id)
		.arg (name)
		.arg (shortName)
		.arg (typeString (type))
		.arg (personRequired?"":"not ")
		;
}

// *******************
// ** SQL interface **
// *******************

QString LaunchMethod::dbTableName ()
{
	return "launch_methods";
}

QString LaunchMethod::selectColumnList ()
{
	return "id,name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments";
}

LaunchMethod LaunchMethod::createFromQuery (const QSqlQuery &q)
{
	LaunchMethod l (q.value (0).toLongLong ());

	l.name                 = q.value (1).toString ();
	l.shortName            = q.value (2).toString ();
	l.logString            = q.value (3).toString ();
	l.keyboardShortcut     = q.value (4).toString ();
	l.type                 = typeFromDb (
	                         q.value (5).toString ());
	l.towplaneRegistration = q.value (6).toString ();
	l.personRequired       = q.value (7).toBool   ();
	l.comments             = q.value (8).toString ();

	return l;
}

QString LaunchMethod::insertValueList ()
{
	return "(name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments) values (?,?,?,?,?,?,?,?)";
}

QString LaunchMethod::updateValueList ()
{
	return "name=?, short_name=?, log_string=?, keyboard_shortcut=?, type=?, towplane_registration=?, person_required=?, comments=?";
}

void LaunchMethod::bindValues (QSqlQuery &q) const
{
	q.addBindValue (name);
	q.addBindValue (shortName);
	q.addBindValue (logString);
	q.addBindValue (keyboardShortcut);
	q.addBindValue (typeToDb (type));
	q.addBindValue (towplaneRegistration);
	q.addBindValue (personRequired);
	q.addBindValue (comments);
}

QList<LaunchMethod> LaunchMethod::createListFromQuery (QSqlQuery &q)
{
	QList<LaunchMethod> list;
	while (q.next ()) list.append (createFromQuery (q));
	return list;
}

// *** Enum mappers
QString LaunchMethod::typeToDb (LaunchMethod::Type type)
{
	switch (type)
	{
		case typeWinch  : return "winch";
		case typeAirtow : return "airtow";
		case typeSelf   : return "self";
		case typeOther  : return "other";
		// no default
	}

	assert (false);
	return "?";
}

LaunchMethod::Type LaunchMethod::typeFromDb (QString type)
{
	if      (type=="winch" ) return typeWinch;
	else if (type=="airtow") return typeAirtow;
	else if (type=="self"  ) return typeSelf;
	else if (type=="other" ) return typeOther;
	else                     return typeOther;
}
