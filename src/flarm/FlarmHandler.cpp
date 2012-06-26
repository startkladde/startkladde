#include "FlarmHandler.h"

#include <QtCore/QTime>
#include <QtCore/QMultiMap>
#include <QtCore/QStringList>
#include <QtCore/QPointF>

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
	regMap (new QMap<QString, FlarmRecord *> ()),
	trace (NULL), stream (NULL),
	enabled (false)
{
	refreshTimer = new QTimer (this);
	connect (refreshTimer, SIGNAL (timeout ()), this, SIGNAL (statusChanged ()));


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
	// FIXME delete trace
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


// **********
// ** Misc **
// **********

void FlarmHandler::setEnabled (bool e) {
        if (e != enabled) {
                enabled = e;
                if (enabled)
                {
        	        // initialize Flarm TcpPort
        	        refreshTimer->start (5000);
                }
                else
                {
                	refreshTimer->stop();
                }
        }
}


QString FlarmHandler::flightActionToString (FlarmRecord::FlightAction action) {
        switch (action)
        {
                case FlarmRecord::departure:
                        return tr ("started");
                case FlarmRecord::landing:
                        return tr ("landed");
                case FlarmRecord::goAround:
                        return tr ("go around");
        }
        return "";
}

void FlarmHandler::updateList (const Plane& plane) {
        if (!plane.flarmId.isEmpty()) {
                FlarmRecord* record = regMap->value (plane.flarmId);
                if (record)
                        record->registration = plane.registration;
        }
}


QDateTime FlarmHandler::getGPSTime () {
        return gpsTime;
}

void FlarmHandler::processPflaaSentence (const PflaaSentence &sentence)
{
	if (!sentence.isValid)
	{
		qDebug () << QString ("Sentence invalid: ")+sentence.sentence;
		return;
	}

	FlarmRecord* record;
	if (!regMap->contains (sentence.flarmId))
	{
		// try get info from flarmnet database
		record = new FlarmRecord (this, sentence.flarmId, FlarmRecord::stateUnknown);
		/*
		 FlarmNetRecord* flarmnet_record = FlarmNetDb::getInstance()->getData (flarmid);
		 if (flarmnet_record) {
		 record->freq = flarmnet_record->freq;
		 record->reg  = flarmnet_record->registration;
		 }
		 */
		// Try to get the registration from own database
		Plane *plane;
		dbId planeId = dbManager->getCache ().getPlaneIdByFlarmId (sentence.flarmId);
		if (idValid (planeId))
		{
			plane = dbManager->getCache ().getNewObject<Plane> (planeId);
			record->registration = plane->registration;
			record->category = plane->category;
		}
		connect (record, SIGNAL(keepAliveTimeout()), this, SLOT(keepAliveTimeout()));
		connect (record, SIGNAL(landingTimeout()), this, SLOT(landingTimeout()));
		connect (record, SIGNAL (actionDetected (const QString &, FlarmHandler::FlightAction)), this, SIGNAL (actionDetected (const QString &, FlarmHandler::FlightAction)));
		regMap->insert (sentence.flarmId, record);
	}
	else
	{
		record = (*regMap)[sentence.flarmId];
	}

	record->processPflaaSentence (sentence);
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
		emit homePosition (QPointF (sentence.longitude.getValue (), sentence.latitude.getValue ()));
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

// FIXME to FlarmRecord
void FlarmHandler::keepAliveTimeout () {
        FlarmRecord* record = (FlarmRecord*)sender();
        if (record) {
                QString flarmid = record->flarmid;
                qDebug () << "keepAliveTimeout:" << flarmid << "; state =" << record->getState();
                record->keepAliveTimer->stop();
                switch (record->getState()) {
                case FlarmRecord::stateLanding:
                        qDebug () << "landing by timeout1:" << flarmid;
                        record->setState (FlarmRecord::stateOnGround);
                        emit actionDetected (flarmid, FlarmRecord::landing);
                        break;
                case FlarmRecord::stateStarting:
                        qDebug () << "out of range:" << flarmid;
                        //don't change state
                        //record->setState (FlarmRecord::stateFlyingFar);
                        break;
                case FlarmRecord::stateFlying:
                        qDebug () << "out of range:" << flarmid;
                        record->setState (FlarmRecord::stateFlyingFar);
                        break;
                default: break;
                }
        }
}

// FIXME to FlarmRecord
void FlarmHandler::landingTimeout () {
        FlarmRecord* record = (FlarmRecord*)sender();
        if (record) {
                QString flarmid = record->flarmid;
                qDebug () << "landingTimeout:" << flarmid << "; state =" << record->getState();
                record->landingTimer->stop();
                switch (record->getState()) {
                case FlarmRecord::stateOnGround:
                        qDebug () << "landing by timeout2:" << flarmid;
                        emit actionDetected (flarmid, FlarmRecord::landing);
                        break;
                default:
                        qCritical () << "landingTimeout in invalid state: " << record->getState() << "; flarmid = " << flarmid;
                        break;
                }
        }
}
