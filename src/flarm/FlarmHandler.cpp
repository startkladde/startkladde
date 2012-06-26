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


QString FlarmHandler::flightActionToString (FlarmHandler::FlightAction action) {
        switch (action)
        {
                case departure:
                        return tr ("started");
                case landing:
                        return tr ("landed");
                case goAround:
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
		regMap->insert (sentence.flarmId, record);
	}
	else
	{
		record = (*regMap)[sentence.flarmId];
	}
	record->keepAliveTimer->start (5000);
	FlarmRecord::flarmState old_state = record->getState ();
	FlarmRecord::flarmEvent event;
	// 10 m/s = 36 km/h
	// FIXME store in m/s
	record->setSpeed ((sentence.groundSpeed * 36) / 10);
	record->north = sentence.relativeNorth;
	record->east = sentence.relativeEast;
	record->setAlt (sentence.relativeVertical);
	record->setClimb (sentence.climbRate);
	if (!record->isPlausible ()) return;

	//                          ground speed
	//           | 0-2 m/s | 2-10 m/s | 10-40 m/s | >40 m/s
	// altitude  |---------+----------+-----------+--------
	// >40m      |ground   |fly       |fly        |fly
	// 0-40m     |ground   |ground    |low        |fly

	if (sentence.groundSpeed < 2)
	{
		// Stationary - on ground
		event = FlarmRecord::eventGround;
	}
	else if (sentence.groundSpeed < 10)
	{
		if (sentence.relativeVertical < 40)
		{
			// Slow with a low altitude - on ground
			event = FlarmRecord::eventGround;
		}
		else
		{
			// Slow with a high altitude - flying
			event = FlarmRecord::eventFly;
		}
	}
	// try to avoid low pass
	else if (sentence.groundSpeed < 40)
	{
		if (sentence.relativeVertical < 40)
		{
			//
			event = FlarmRecord::eventLow;
		}
		else
		{
			event = FlarmRecord::eventFly;
		}
	}
	else
	{
		event = FlarmRecord::eventFly;
	}

	switch (old_state)
	{
		case FlarmRecord::stateOnGround:
			switch (event)
			{
				case FlarmRecord::eventFly:
					// should not happen. Plane was on ground before, now flying?
					// ignore event?
					if (record->landingTimer->isActive ())
					{
						qCritical () << "unexpected go around1: " << sentence.flarmId;
						record->setState (FlarmRecord::stateFlying);
						record->landingTimer->stop ();
						if (record->category == Plane::categoryMotorglider)
							emit actionDetected (sentence.flarmId, FlarmHandler::goAround);
						else
							qCritical () << "unexpected goaround of glider?";
					}
					else
					{
						qCritical () << "unexpected start: " << sentence.flarmId;
						record->setState (FlarmRecord::stateFlying);
						emit actionDetected (sentence.flarmId, FlarmHandler::departure);
					}
					break;
				case FlarmRecord::eventLow:
					if (record->landingTimer->isActive ())
					{
						qDebug () << "go around2:" << sentence.flarmId;
						record->setState (FlarmRecord::stateStarting);
						record->landingTimer->stop ();
						if (record->category == Plane::categoryMotorglider)
							emit actionDetected (sentence.flarmId, FlarmHandler::goAround);
						else
							qCritical () << "unexpected goaround of glider?";
					}
					else
					{
						record->setState (FlarmRecord::stateStarting);
						qDebug () << "flat start:" << sentence.flarmId;
						emit actionDetected (sentence.flarmId, FlarmHandler::departure);
					}
					break;
				default:
					;
			}
			break;
		case FlarmRecord::stateLanding:
			switch (event)
			{
				case FlarmRecord::eventGround:
					qDebug () << "landing continued:" << sentence.flarmId;
					record->setState (FlarmRecord::stateOnGround);
					if (record->category == Plane::categoryMotorglider)
						record->landingTimer->start (30000);
					else
						emit actionDetected (sentence.flarmId, FlarmHandler::landing);
					break;
				case FlarmRecord::eventFly:
					qDebug () << "go around3:" << sentence.flarmId;
					record->setState (FlarmRecord::stateFlying);
					if (record->category == Plane::categoryMotorglider)
						emit actionDetected (sentence.flarmId, FlarmHandler::goAround);
					else
						qCritical () << "unexpected goaround of glider?";
					break;
				default:
					;
			}
			break;
		case FlarmRecord::stateFlying:
			switch (event)
			{
				case FlarmRecord::eventGround:
					qCritical () << "unexpected landing from high: " << sentence.flarmId;
					record->setState (FlarmRecord::stateOnGround);
					if (record->category == Plane::categoryMotorglider)
						record->landingTimer->start (30000);
					else
						emit actionDetected (sentence.flarmId, FlarmHandler::landing);
					break;
				case FlarmRecord::eventLow:
					qDebug () << "flying low:" << sentence.flarmId;
					record->setState (FlarmRecord::stateLanding);
					break;
				default:
					;
			}
			break;
		case FlarmRecord::stateFlyingFar:
			switch (event)
			{
				case FlarmRecord::eventGround:
					qCritical () << "unexpected landing from far: " << sentence.flarmId;
					record->setState (FlarmRecord::stateOnGround);
					if (record->category == Plane::categoryMotorglider)
						record->landingTimer->start (30000);
					else
						emit actionDetected (sentence.flarmId, FlarmHandler::landing);
					break;
				case FlarmRecord::eventLow:
					qDebug () << "flying low:" << sentence.flarmId;
					record->setState (FlarmRecord::stateLanding);
					break;
				case FlarmRecord::eventFly:
					qDebug () << "still flying:" << sentence.flarmId;
					record->setState (FlarmRecord::stateFlying);
					break;
				default:
					;
			}
			break;
		case FlarmRecord::stateStarting:
			switch (event)
			{
				case FlarmRecord::eventGround:
					qDebug () << "departure aborted:" << sentence.flarmId;
					record->setState (FlarmRecord::stateOnGround);
					emit actionDetected (sentence.flarmId, FlarmHandler::landing);
					break;
				case FlarmRecord::eventFly:
					qDebug () << "departure continued:" << sentence.flarmId;
					record->setState (FlarmRecord::stateFlying);
					break;
				default:
					;
			}
			break;
		default:
			switch (event)
			{
				case FlarmRecord::eventGround:
					qDebug () << "new on ground:" << sentence.flarmId;
					record->setState (FlarmRecord::stateOnGround);
					break;
				case FlarmRecord::eventFly:
					qDebug () << "new flying:" << sentence.flarmId;
					record->setState (FlarmRecord::stateFlying);
					break;
				case FlarmRecord::eventLow:
					qDebug () << "new flying low:" << sentence.flarmId;
					record->setState (FlarmRecord::stateLanding);
					break;
				default:
					;
			}
			break;
	}
	if (old_state != record->getState ()) emit statusChanged ();
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
                        emit actionDetected (flarmid, FlarmHandler::landing);
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
                default:
                        ;
                }
        }
}

void FlarmHandler::landingTimeout () {
        FlarmRecord* record = (FlarmRecord*)sender();
        if (record) {
                QString flarmid = record->flarmid;
                qDebug () << "landingTimeout:" << flarmid << "; state =" << record->getState();
                record->landingTimer->stop();
                switch (record->getState()) {
                case FlarmRecord::stateOnGround:
                        qDebug () << "landing by timeout2:" << flarmid;
                        emit actionDetected (flarmid, FlarmHandler::landing);
                        break;
                default:
                        qCritical () << "landingTimeout in invalid state: " << record->getState() << "; flarmid = " << flarmid;
                }
        }
}
