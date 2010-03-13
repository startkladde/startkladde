#include "LaunchMethod.h"

#include <cassert>

#include <QVariant>

#include "src/logging/messages.h"
#include "src/util/bool.h"
#include "src/db/result/Result.h"
#include "src/db/Query.h"
#include "src/util/qString.h"


// ******************
// ** Construction **
// ******************

LaunchMethod::LaunchMethod ()
	:Entity ()
{
	initialize ();
}

LaunchMethod::LaunchMethod (dbId id):
	Entity (id)
{
	initialize ();
}

// Class management
void LaunchMethod::initialize ()
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
	LaunchMethod launchMethod;

	// Split the line and simplify whitespace
	QStringList split=line.split (",");
	for (int i=0; i<split.size (); ++i)
		split[i]=split[i].simplifyWhiteSpace ();

	// Set the attributes from the split parts
	int n=split.count ();
	if (n>=0) launchMethod.id=split[0].toLongLong ();
	if (n>=1)
	{
		if      (split[1].lower ()=="winch" ) launchMethod.type=typeWinch;
		else if (split[1].lower ()=="airtow") launchMethod.type=typeAirtow;
		else if (split[1].lower ()=="self"  ) launchMethod.type=typeSelf;
		else if (split[1].lower ()=="other" ) launchMethod.type=typeOther;
		else
		{
			log_error ("Unknown launch method type in LaunchMethod::parseConfigLine (QString)");
			launchMethod.type=typeOther;
		}
	}
	else
	{
		launchMethod.type=typeOther;
	}
	if (n>=2) launchMethod.towplaneRegistration=split[2];
	if (n>=3) launchMethod.name=split[3];
	if (n>=4) launchMethod.shortName=split[4];
	if (n>=5) launchMethod.keyboardShortcut=split[5];
	if (n>=6) launchMethod.logString=split[6];
	if (n>=7 && split[7].lower ()=="false") launchMethod.personRequired=false;

	return launchMethod;
}


// ****************
// ** Formatting **
// ****************

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

QString LaunchMethod::nameWithShortcut () const
{
	if (keyboardShortcut.isEmpty ())
		return QString (" ")+name;
	else
		return QString (keyboardShortcut)+QString (" - ")+name;
}


// ******************
// ** Type methods **
// ******************

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

QList<LaunchMethod::Type> LaunchMethod::listTypes ()
{
	return QList<Type> ()
		<< typeWinch
		<< typeAirtow
		<< typeSelf
		<< typeOther
		;
}


// ********************************
// ** EntitySelectWindow helpers **
// ********************************

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


// *****************
// ** ObjectModel **
// *****************

int LaunchMethod::DefaultObjectModel::columnCount () const
{
	return 9;
}

QVariant LaunchMethod::DefaultObjectModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return utf8 ("Name");
		case 1: return utf8 ("Kürzel");
		case 2: return utf8 ("Flugbuch-Bezeichnung");
		case 3: return utf8 ("Tastenkürzel");
		case 4: return utf8 ("Typ");
		case 5: return utf8 ("Schleppflugzeug");
		case 6: return utf8 ("Person erforderlich");
		case 7: return utf8 ("Bemerkungen");
		// TODO remove from DefaultObjectModel?
		case 8: return "ID";
	}

	assert (false);
	return QVariant ();
}

QVariant LaunchMethod::DefaultObjectModel::displayData (const LaunchMethod &object, int column) const
{
	switch (column)
	{
		case 0: return object.name;
		case 1: return object.shortName;
		case 2: return object.logString;
		case 3: return object.keyboardShortcut;
		case 4: return typeString (object.type);
		case 5: return object.isAirtow ()?object.towplaneRegistration:"-";
		case 6: return boolToString (object.personRequired);
		case 7: return object.comments;
		case 8: return object.id;
	}

	assert (false);
	return QVariant ();
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

LaunchMethod LaunchMethod::createFromResult (const Result &result)
{
	LaunchMethod l (result.value (0).toLongLong ());

	l.name                 = result.value (1).toString ();
	l.shortName            = result.value (2).toString ();
	l.logString            = result.value (3).toString ();
	l.keyboardShortcut     = result.value (4).toString ();
	l.type                 = typeFromDb (
	                         result.value (5).toString ());
	l.towplaneRegistration = result.value (6).toString ();
	l.personRequired       = result.value (7).toBool   ();
	l.comments             = result.value (8).toString ();

	return l;
}

QString LaunchMethod::insertColumnList ()
{
	return "name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments";
}

QString LaunchMethod::insertPlaceholderList ()
{
	return "?,?,?,?,?,?,?,?";
}

void LaunchMethod::bindValues (Query &q) const
{
	q.bind (name);
	q.bind (shortName);
	q.bind (logString);
	q.bind (keyboardShortcut);
	q.bind (typeToDb (type));
	q.bind (towplaneRegistration);
	q.bind (personRequired);
	q.bind (comments);
}

QList<LaunchMethod> LaunchMethod::createListFromResult (Result &result)
{
	QList<LaunchMethod> list;
	while (result.next ()) list.append (createFromResult (result));
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
