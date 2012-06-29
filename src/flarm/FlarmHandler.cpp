#include "FlarmHandler.h"

#include <QtCore/QTime>
#include <QtCore/QMultiMap>
#include <QtCore/QStringList>
#include <QtCore/QPointF>

#include "src/model/objectList/MutableObjectList.h"
#include "src/config/Settings.h"
#include "src/model/Plane.h"
#include "src/io/dataStream/TcpDataStream.h"
#include "src/flarm/Nmea.h"
#include "src/flarm/GprmcSentence.h"
#include "src/flarm/PflaaSentence.h"


// ****************************
// ** Construction/singleton **
// ****************************

FlarmHandler* FlarmHandler::instance = NULL;

FlarmHandler::FlarmHandler (QObject* parent): QObject (parent),
//	regMap (new QMap<QString, FlarmRecord *> ()),
	trace (NULL), stream (NULL),
	flarmRecords (new MutableObjectList<FlarmRecord *>)
{
//	QDate today (QDate::currentDate());
//	QString filename = QString ("/var/log/startkladde/startkladde-%1.trc").arg(today.toString("yyyyMMdd"));
//	trace = new QFile (filename, this);
//	trace->open (QIODevice::Append);
//	stream = new QTextStream (trace);
}

FlarmHandler::~FlarmHandler ()
{
	if (trace) trace->close();
	delete trace;
	delete stream;
	delete flarmRecords;
}

FlarmHandler* FlarmHandler::getInstance ()
{
	if (!instance)
		instance = new FlarmHandler (NULL);
        return instance;
}


// ****************
// ** Properties **
// ****************

void FlarmHandler::setDatabase (DbManager *db)
{
	dbManager = db;
}



QDateTime FlarmHandler::getGpsTime () {
        return gpsTime;
}


// ****************
// ** Flarm data **
// ****************

const MutableObjectList<FlarmRecord *> *FlarmHandler::getFlarmRecords () const
{
	return flarmRecords;
}

int FlarmHandler::findFlarmRecordByFlarmId (const QString &flarmId)
{
	// FIXME cache - listen to the MutableObjectList signals
	for (int i=0, n=flarmRecords->size (); i<n; ++i)
		if (flarmRecords->at (i)->getFlarmId ()==flarmId)
			return i;

	return -1;
}

// **********
// ** Misc **
// **********

int FlarmHandler::findOrCreateFlarmRecord (const QString &flarmId)
{
	int index=findFlarmRecordByFlarmId (flarmId);

	if (index>=0)
	{
		return index;
	}
	else
	{
		// FIXME delete it
		FlarmRecord *record=new FlarmRecord (this, flarmId);

		// try get info from flarmnet database
		/*
		 FlarmNetRecord* flarmnet_record = FlarmNetDb::getInstance()->getData (flarmid);
		 if (flarmnet_record) {
		 record->freq = flarmnet_record->freq;
		 record->reg  = flarmnet_record->registration;
		 }
		 */

		// Try to get the registration from own database
		Plane *plane;
		dbId planeId = dbManager->getCache ().getPlaneIdByFlarmId (flarmId);
		if (idValid (planeId))
		{
			plane = dbManager->getCache ().getNewObject<Plane> (planeId);
			record->setRegistration (plane->registration);
			record->setCategory (plane->category);
		}

		connect (record, SIGNAL (actionDetected (const QString &, FlarmRecord::FlightAction)), this, SIGNAL (actionDetected (const QString &, FlarmRecord::FlightAction)));

		flarmRecords->append (record);
		return flarmRecords->size ()-1;
	}

}

void FlarmHandler::processPflaaSentence (const PflaaSentence &sentence)
{
	if (!sentence.isValid)
	{
		qDebug () << QString ("Sentence invalid: ")+sentence.sentence;
		return;
	}

	// FIXME DOING do properly:
	//   * modifying the MutlableObjectList entry
	//   * then caching of flarm id

	int index=findOrCreateFlarmRecord (sentence.flarmId);

	// FIXME the flarm record may also update "itself" from its timer event. The
	// model will not notice that.
	flarmRecords->at (index)->processPflaaSentence (sentence);
	flarmRecords->update (index);

	emit statusChanged ();
}

void FlarmHandler::processGprmcSentence (const GprmcSentence &sentence)
{
	if (!sentence.isValid)
	{
		qDebug () << QString ("Sentence invalid: ")+sentence.sentence;
		return;
	}

	if (sentence.status)
	{
		// FIXME store, and use GeoPosition
		emit homePosition (sentence.position);
		// FIXME signal
		gpsTime=sentence.timestamp;
	}
}

void FlarmHandler::lineReceived (const QString &line)
{
	//	qDebug () << "Flarm handler: process sentence " << line;

	if (stream) *stream << line;

	// TODO also process GPGGA and GPGSA/GPGSV sentences
	if (line.startsWith ("$PFLAA"))
		processPflaaSentence (PflaaSentence (line));
	else if (line.startsWith ("$GPRMC"))
		processGprmcSentence (GprmcSentence (line));
}
