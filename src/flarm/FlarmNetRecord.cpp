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

QString FlarmNetRecord::getFlarmId () const {
	return flarmId;
}

QString FlarmNetRecord::getRegistration () const {
	return registration;
}

QString FlarmNetRecord::getOwner () const {
	return owner;
}

QString FlarmNetRecord::getType () const {
	return type;
}

QString FlarmNetRecord::getCallsign () const {
	return callsign;
}

QString FlarmNetRecord::getFrequency () const {
	return frequency;
}

void FlarmNetRecord::setFlarmId (const QString& _flarmId) {
	flarmId = _flarmId.trimmed();
}

void FlarmNetRecord::setRegistration (const QString& _registration) {
	registration = _registration.trimmed();
}

void FlarmNetRecord::setOwner (const QString& _owner) {
	owner = _owner.trimmed();
}

void FlarmNetRecord::setType  (const QString& _type) {
	type = _type.trimmed();
}

void FlarmNetRecord::setCallsign (const QString& _callsign) {
	callsign = _callsign.trimmed();
}

void FlarmNetRecord::setFrequency (const QString& _frequency) {
	frequency = _frequency.trimmed();
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

	p.setFlarmId      (result.value (1).toString ());
	p.setRegistration (result.value (2).toString ());
	p.setCallsign     (result.value (3).toString ());
	p.setOwner        (result.value (4).toString ());
	p.setType         (result.value (5).toString ());
	p.setFrequency    (result.value (6).toString ());
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
