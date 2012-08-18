#include "FlarmNetRecord.h"

#include <cassert>

#include <QApplication>
#include <QtCore/QDebug>

#include "src/text.h"
#include "src/db/result/Result.h"
#include "src/db/Query.h"
#include "src/i18n/notr.h"

// ******************
// ** Construction **
// ******************

FlarmNetRecord::FlarmNetRecord ():
        Entity ()
{
	initialize ();
}

FlarmNetRecord::FlarmNetRecord (dbId id):
        Entity (id)
{
	initialize ();
}

void FlarmNetRecord::initialize ()
{
}


// ****************
// ** Formatting **
// ****************

QString FlarmNetRecord::toString () const
{
	return qnotr ("id=%1, flarm_id=%2, registration=%3, callsign=%4, owner=%5, type=%6, frequency=%7")
		.arg (id)
		.arg (flarmId)
		.arg (registration)
		.arg (callsign)
		.arg (owner)
		.arg (type)
		.arg (frequency)
		;
}

QString FlarmNetRecord::getDisplayName () const
{
	return flarmId;
}


// *******************
// ** SQL interface **
// *******************

QString FlarmNetRecord::dbTableName ()
{
	return notr ("flarmnet");
}

QString FlarmNetRecord::selectColumnList ()
{
	return notr ("id,flarm_id,registration,callsign,owner,type,frequency");
}


FlarmNetRecord FlarmNetRecord::createFromResult (const Result &result)
{
	FlarmNetRecord p (result.value (0).toLongLong ());

	p.flarmId      = result.value (1).toString ();
	p.registration = result.value (2).toString ();
	p.callsign     = result.value (3).toString ();
	p.owner        = result.value (4).toString ();
	p.type         = result.value (5).toString ();
	p.frequency    = result.value (6).toString ();
	// qDebug () << "FlarmNetRecord::createFromResult: " << p.toString() << endl;

	return p;
}

QString FlarmNetRecord::insertColumnList ()
{
	return notr ("flarm_id,registration,callsign,owner,type,frequency");
}

QString FlarmNetRecord::insertPlaceholderList ()
{
	return notr ("?,?,?,?,?,?");
}

void FlarmNetRecord::bindValues (Query &q) const
{
	q.bind (flarmId);
	q.bind (registration);
	q.bind (callsign);
	q.bind (owner);
	q.bind (type);
	q.bind (frequency);
}

QList<FlarmNetRecord> FlarmNetRecord::createListFromResult (Result &result)
{
	QList<FlarmNetRecord> list;

	while (result.next ())
		list.append (createFromResult (result));

	return list;
}


// ****************
// ** Conversion **
// ****************

Plane FlarmNetRecord::toPlane () const
{
	Plane plane;

	plane.registration=registration;
	plane.club=owner;
	plane.numSeats=-1;
	plane.type=type;
	plane.category=Plane::categoryFromRegistration (registration);
	plane.callsign=callsign;
	plane.flarmId=flarmId;

	return plane;
}
