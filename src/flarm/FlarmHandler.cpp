/**
 * FlarmHandler.cpp
 * This program receives serial data via an UNIX socket from a Flarm device and tries to
 * analyse the state of each glider that sends flarm data to create 
 * start and landing events for further processing
 * 
 * written by Eggert Ehmke 
 * eggert.ehmke@ftv-spandau.de
 * Copyright (C) 2012 Eggert Ehmke
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses.
 */

#include "FlarmHandler.h"

#include <QtCore/QTime>
#include <QtCore/QMultiMap>
#include <QtCore/QStringList>
#include <QtCore/QPointF>

#include "src/config/Settings.h"
#include "src/model/Plane.h"
#include "src/io/dataStream/TcpDataStream.h"
#include "src/flarm/Nmea.h"


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

double FlarmHandler::calcLat (const QString& lat, const QString& ns) {
        QString deg = lat.left(2);
        QString min = lat.mid(2);
        double value = deg.toDouble() + min.toDouble() / 60.0;
        if (ns == "S") 
                value = -value;
        return value;
}

double FlarmHandler::calcLon (const QString& lon, const QString& ew) {
        QString deg = lon.left(3);
        QString min = lon.mid(3);
        double value = deg.toDouble() + min.toDouble() / 60.0;
        if (ew == "W") 
                value = -value;
        return value;
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

void FlarmHandler::processFlarm (const QString& line) {
//	qDebug () << "Flarm handler: process sentence " << line;

        if (!Nmea::verifyChecksum (line)) {
                //qDebug () << "Checksum failed: " << line << endl; 
                return;
        }
        if (stream) *stream << line;
	QStringList list = line.split (',');
	QString sentence = list [0];
	if (sentence == "$PFLAA") {
		// the list can be cut off when serial transfer is interrupted
		// qDebug () << line << endl;
		if (list.length() < 10) {
			qDebug () << "Sentence interrupted: " << line << endl;
			return;
		}
		int RelativeNorth = list [2].toInt();
		int RelativeEast = list [3].toInt();
		int RelativeVertical = list [4].toInt();
		QString flarmid = list[6];
		int GroundSpeed = list [9].toInt();
		double ClimbRate = list [10].toDouble();
		//if (RelativeVertical > 100) {
		//        qDebug () << "vario: " << list [10] << list [10].toDouble() << endl;
                //}
	        FlarmRecord* record;
	        if (!regMap->contains(flarmid)) {
	                // try get info from flarmnet database
		        record = new FlarmRecord (this, flarmid, FlarmRecord::stateUnknown);
	                /*
	                FlarmNetRecord* flarmnet_record = FlarmNetDb::getInstance()->getData (flarmid);
	                if (flarmnet_record) {
	                        record->freq = flarmnet_record->freq;
	                        record->reg  = flarmnet_record->registration;
	                }
	                */
		        // Try to get the registration from own database
		        Plane *plane;
		        dbId planeId = dbManager->getCache().getPlaneIdByFlarmId (flarmid);
                        if (idValid (planeId)) {
                                plane = dbManager->getCache().getNewObject<Plane> (planeId);
                                record->registration = plane->registration;
                                record->category = plane->category;
                        } 
		        connect (record, SIGNAL(keepAliveTimeout()), this, SLOT(keepAliveTimeout()));
		        connect (record, SIGNAL(landingTimeout()), this, SLOT(landingTimeout()));
	        	regMap->insert (flarmid, record);
                }
		else {
			record = (*regMap) [flarmid];
		}
		record->keepAliveTimer->start (5000);
		FlarmRecord::flarmState old_state = record->getState();
		FlarmRecord::flarmEvent event;
		// 10 m/s = 36 km/h
		record->setSpeed ((GroundSpeed * 36) / 10);
		record->north = RelativeNorth;
		record->east = RelativeEast;
		record->setAlt (RelativeVertical);
		record->setClimb (ClimbRate);
		if (!record->isPlausible())
		        return;
		        
		// print "GroundSpeed: ", GroundSpeed
		if (GroundSpeed < 2) {
		        event = FlarmRecord::eventGround;
		}
		else if (GroundSpeed < 10) {
		        if (RelativeVertical < 40) {
			        // print reg, "slow"
			        event = FlarmRecord::eventGround;
                        }
                        else {
                                event = FlarmRecord::eventFly;
                        }
		}
		// try to avoid low pass
		else if (GroundSpeed < 40) {
		        if (RelativeVertical < 40) {
                                event = FlarmRecord::eventLow;
                        }
                        else {
                                event = FlarmRecord::eventFly;
                        }                                
		}
		else {
                        event = FlarmRecord::eventFly;
		}
		
		switch (old_state) {
		case FlarmRecord::stateOnGround:
			switch (event) {
			case FlarmRecord::eventFly:
		                // should not happen. Plane was on ground before, now flying?
		                // ignore event? 
			        if (record->landingTimer->isActive()) {
        			        qCritical () << "unexpected go around1: " << flarmid;
                                	record->setState (FlarmRecord::stateFlying);
                                	record->landingTimer->stop();
                                	if (record->category == Plane::categoryMotorglider)
		        	                emit actionDetected (flarmid, FlarmHandler::goAround);
                                        else
                                                qCritical () << "unexpected goaround of glider?" << endl;
			        }
			        else {
                        	        qCritical () << "unexpected start: " << flarmid;
                        	        record->setState (FlarmRecord::stateFlying);
                        	        emit actionDetected (flarmid, FlarmHandler::departure);
                        	}
                        	break;
			case FlarmRecord::eventLow:
			        if (record->landingTimer->isActive()) {
        			        qDebug () << "go around2: " << flarmid;
                                	record->setState (FlarmRecord::stateStarting);
                                	record->landingTimer->stop();
                                	if (record->category == Plane::categoryMotorglider)
		        	                emit actionDetected (flarmid, FlarmHandler::goAround);
                                        else
                                                qCritical () << "unexpected goaround of glider?";
			        }
			        else {
			                record->setState (FlarmRecord::stateStarting);
			                qDebug () << "flat start: " << flarmid;
			                emit actionDetected (flarmid, FlarmHandler::departure);
                                }
                        	break;
                        default:
                                ;
			}
			break;
		case FlarmRecord::stateLanding:
			switch (event) {
			case FlarmRecord::eventGround: 
                               	qDebug () << "landing continued: " << flarmid;
                               	record->setState (FlarmRecord::stateOnGround);
                        	if (record->category == Plane::categoryMotorglider)
                        	        record->landingTimer->start(30000);
                                else
                        	        emit actionDetected (flarmid, FlarmHandler::landing);
                        	break;
			case FlarmRecord::eventFly:
			        qDebug () << "go around3: " << flarmid;
                        	record->setState (FlarmRecord::stateFlying);
                        	if (record->category == Plane::categoryMotorglider)
        			        emit actionDetected (flarmid, FlarmHandler::goAround);
                                else
                                        qCritical () << "unexpected goaround of glider?";
			        break;
                        default:
                                ;
			}
			break;
		case FlarmRecord::stateFlying:
			switch (event) {
			case FlarmRecord::eventGround:
                               	qCritical () << "unexpected landing from high: " << flarmid;
                        	record->setState (FlarmRecord::stateOnGround);
                        	if (record->category == Plane::categoryMotorglider)
                        	        record->landingTimer->start(30000);
                                else
                                	emit actionDetected (flarmid, FlarmHandler::landing);
                        	break;
                        case FlarmRecord::eventLow:
                               	qDebug () << "flying low: " << flarmid;
                                record->setState (FlarmRecord::stateLanding);
                                break;
                        default:
                                ;
			}
			break;
		case FlarmRecord::stateFlyingFar:
			switch (event) {
			case FlarmRecord::eventGround:
                               	qCritical () << "unexpected landing from far: " << flarmid;
                        	record->setState (FlarmRecord::stateOnGround);
                        	if (record->category == Plane::categoryMotorglider)
                        	        record->landingTimer->start(30000);
                                else
                                	emit actionDetected (flarmid, FlarmHandler::landing);
                        	break;
                        case FlarmRecord::eventLow:
                               	qDebug () << "flying low: " << flarmid;
                                record->setState (FlarmRecord::stateLanding);
                                break;
                        case FlarmRecord::eventFly:
                               	qDebug () << "still flying: " << flarmid;
                                record->setState (FlarmRecord::stateFlying);
                                break;
                        default:
                                ;
			}
			break;
		case FlarmRecord::stateStarting:
			switch (event) {
			case FlarmRecord::eventGround: 
                               	qDebug () << "departure aborted: " << flarmid;
                        	record->setState (FlarmRecord::stateOnGround);
                        	emit actionDetected (flarmid, FlarmHandler::landing);
                        	break;
			case FlarmRecord::eventFly:
                               	qDebug () << "departure continued: " << flarmid;
                        	record->setState (FlarmRecord::stateFlying);
			        break;
                        default:
                                ;
			}
			break;
                default:
			switch (event) {
			case FlarmRecord::eventGround: 
                               	qDebug () << "new on ground: " << flarmid;
                        	record->setState (FlarmRecord::stateOnGround);
                        	break;
			case FlarmRecord::eventFly:
                               	qDebug () << "new flying: " << flarmid;
                        	record->setState (FlarmRecord::stateFlying);
			        break;
			case FlarmRecord::eventLow:
                               	qDebug () << "new flying low: " << flarmid;
                        	record->setState (FlarmRecord::stateLanding);
			        break;
                        default:
                                ;
			}
			break;
                }
                if (old_state != record->getState())
                        emit statusChanged ();
	}
	else if (sentence == "$GPRMC") {
	        QString timestamp = list[1];
	        QString valid     = list[2];
	        QString lat       = list[3];
	        QString ns        = list[4];
	        QString lon       = list[5];
	        QString ew        = list[6];
	        QString datestamp = list[9];
	        // qDebug () << "raw time: " << timestamp;
	        QTime time (QTime::fromString (timestamp.split('.')[0], "hhmmss"));
	        QDate date (QDate::fromString (datestamp, "ddMMyy"));
	        // two digit year give 19xx years. We have to add 100 years :-/
	        gpsTime = QDateTime (date.addYears(100), time, Qt::UTC);
	        //qDebug () << "datetime: " << gpsTime.toString ("hh:mm:ss dd.MM.yy");
	        if (valid == "A") {
        	        double dlat = calcLat (lat, ns);
        	        double dlon = calcLon (lon, ew);
        	        emit homePosition (QPointF (dlon, dlat));
                }
	}
}

void FlarmHandler::keepAliveTimeout () {
        FlarmRecord* record = (FlarmRecord*)sender();
        if (record) {
                QString flarmid = record->flarmid;
                qDebug () << "keepAliveTimeout: " << flarmid << "; state = " << record->getState();
                record->keepAliveTimer->stop();
                switch (record->getState()) {
                case FlarmRecord::stateLanding:
                        qDebug () << "landing by timeout1: " << flarmid << endl;
                        record->setState (FlarmRecord::stateOnGround);
                        emit actionDetected (flarmid, FlarmHandler::landing);
                        break;
                case FlarmRecord::stateStarting:
                        qDebug () << "out of range: " << flarmid << endl;
                        //don't change state
                        //record->setState (FlarmRecord::stateFlyingFar);
                        break;
                case FlarmRecord::stateFlying:
                        qDebug () << "out of range: " << flarmid << endl;
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
                qDebug () << "landingTimeout: " << flarmid << "; state = " << record->getState() << endl;
                record->landingTimer->stop();
                switch (record->getState()) {
                case FlarmRecord::stateOnGround:
                        qDebug () << "landing by timeout2: " << flarmid << endl;
                        emit actionDetected (flarmid, FlarmHandler::landing);
                        break;
                default:
                        qCritical () << "landingTimeout in invalid state: " << record->getState() << "; flarmid = " << flarmid << endl;
                }
        }
}

void FlarmHandler::lineReceived (const QString &line)
{
	processFlarm (line);
}
