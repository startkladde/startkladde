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
#include <QtCore/QTime>
#include <QtCore/QMultiMap>
#include <QtCore/QStringList>
#include <QtCore/QPointF>

#include "src/config/Settings.h"
#include "src/model/Plane.h"
#include "FlarmHandler.h"

FlarmHandler* FlarmHandler::instance = NULL;

FlarmHandler::FlarmHandler (QObject* parent) 
  :QObject (parent)
{
        regMap = NULL;
        flarmDevice = new QTcpSocket (this);
        // force to emit signal
        connectionState = connectedNoData;
        setConnectionState (notConnected);
        flarmDataTimer = new QTimer (this);
        flarmDeviceTimer = new QTimer (this);
        refreshTimer = new QTimer (this);
        connect (flarmDataTimer, SIGNAL(timeout()), this, SLOT(flarmDataTimeout()));
        connect (flarmDeviceTimer, SIGNAL(timeout()), this, SLOT(flarmDeviceTimeout()));
        connect (refreshTimer, SIGNAL(timeout()), this, SIGNAL(statusChanged()));
        connect (flarmDevice, SIGNAL(readyRead()), this, SLOT(dataReceived()));
        connect (flarmDevice, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketException(QAbstractSocket::SocketError)));
        connect (flarmDevice, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));

        regMap = new QMultiMap <QString, FlarmRecord*> ();

        enabled = false;
        setEnabled (Settings::instance().flarmEnabled);

	QDate today (QDate::currentDate());
	QString filename = QString ("/var/log/startkladde/startkladde-%1.trc").arg(today.toString("yyyyMMdd"));
	trace = new QFile (filename, this);
	trace->open (QIODevice::Append);
	stream = new QTextStream (trace);
}

FlarmHandler::~FlarmHandler () {
        trace->close();
        delete regMap;
        delete instance;
}

void FlarmHandler::setEnabled (bool e) {
        if (e != enabled) {
                enabled = e;
                if (enabled)
                {
        	        // initialize Flarm TcpPort
        	        QTimer::singleShot (0, this, SLOT(initFlarmDevice()));
        	        refreshTimer->start (5000);
                }
                else {
                        flarmDevice->close();
                        refreshTimer->stop();
                        flarmDeviceTimer->stop();
                        flarmDataTimer->stop();
                        setConnectionState (notConnected);
                }
        }
}

FlarmHandler* FlarmHandler::getInstance () {
        if (instance == NULL)
                instance = new FlarmHandler (NULL);
        return instance;
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

void FlarmHandler::setDatabase (DbManager* db) {
        dbManager = db;
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

void FlarmHandler::socketStateChanged (QAbstractSocket::SocketState socketState) {
        if (socketState == QAbstractSocket::ConnectedState)
                setConnectionState (connectedNoData);
        else
                setConnectionState (notConnected);
}

void FlarmHandler::setConnectionState (ConnectionState state) {
        if (state != connectionState) {
                connectionState = state;
                emit connectionStateChanged (state);
        }
}

FlarmHandler::ConnectionState FlarmHandler::getConnectionState () {
        return connectionState;
}

QDateTime FlarmHandler::getGPSTime () {
        return gpsTime;
}
        
void FlarmHandler::initFlarmDevice () {
	// will be set on first sentence
	setConnectionState (notConnected);
	
	//qDebug () << "FlarmHandler::initFlarmDevice: " << endl;

	if (flarmDevice->state() == QAbstractSocket::ConnectedState) {
		qDebug () << "FlarmSocket is connected" << endl;
		// stop the timer. As long as no exception, no timer.
		flarmDeviceTimer->stop ();
		return;
	}
 	else if (flarmDevice->state() == QAbstractSocket::ConnectingState) {
 	        // this does not seem to happen
		qDebug () << "FlarmSocket is connecting" << endl;
		flarmDeviceTimer->stop ();
		flarmDevice->abort ();
		return;
 	}
 	else
 	        //qDebug() << "state: " << flarmDevice->state() << endl;
	
        //qDebug () << "try to connect flarm socket" << endl;
	flarmDevice->connectToHost ("localhost", 4711, QIODevice::ReadOnly);
	flarmDataTimer->start (2000);
}

void FlarmHandler::dataReceived () {
	//qDebug () << "dataReceived" << endl;
	setConnectionState (connectedData);
	flarmDataTimer->start(2000);
	while (flarmDevice->canReadLine()) {
		QString line = flarmDevice->readLine ();
		// qDebug () << "line: " << line << endl;
		processFlarm (line);
	}
}

void FlarmHandler::socketException (QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError)
	qDebug () << "socketException: " << socketError << endl;
	flarmDevice->abort();
	setConnectionState (notConnected);
	flarmDeviceTimer->start (5000);
	flarmDataTimer->stop ();
}

void FlarmHandler::flarmDataTimeout () {
	qDebug () << "FlarmHandler::flarmDataTimeout" << endl;
	setConnectionState (connectedNoData);
	// if no data, most likely the socket is down. Give it enough time to recover     
	flarmDataTimer->start(6000);
}

void FlarmHandler::flarmDeviceTimeout () {
	qDebug () << "FlarmHandler::flarmDeviceTimeout" << endl;
        // do not start timer; initFlarmSocket will take care   
	initFlarmDevice ();
}

/** This function calculates the checksum in the sentence.
   
   NMEA-0183 Standard
   The optional checksum field consists of a "*" and two hex digits
   representing the exclusive OR of all characters between, but not
   including, the "$" and "*".  A checksum is required on some sentences.
*/
uint FlarmHandler::calcCheckSum (const QString& sentence)  
{
        uchar sum = 0;
        uchar len = sentence.length();

        for( int i=1; i < len; i++ )
        {
                uchar c = (sentence[i]).toAscii();

                if( c == '$' ) // Start sign will not be considered
                        continue;

                if( c == '*' ) // End of sentence reached
                        break;
                                             
                sum ^= c;
        }

        return sum;
}

/** This function checks if the checksum in the sentence matches the sentence. It returns true if it matches, and false otherwise. */
bool FlarmHandler::checkCheckSum(const QString& sentence)
{
        QStringList list = sentence.split ('*');
        if (list.length() > 1) {
                uchar check = (uchar) list[1].trimmed().toUShort(0, 16);
                return (check == calcCheckSum (list[0]));
        }
        else
                return false;
}

void FlarmHandler::processFlarm (const QString& line) {
        if (!checkCheckSum (line)) {
                //qDebug () << "Checksum failed: " << line << endl; 
                return;
        }
        *stream << line;
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
        			        qCritical () << "unexpected go around1: " << flarmid << endl;
                                	record->setState (FlarmRecord::stateFlying);
                                	record->landingTimer->stop();
                                	if (record->category == Plane::categoryMotorglider)
		        	                emit actionDetected (flarmid, FlarmHandler::goAround);
                                        else
                                                qCritical () << "unexpected goaround of glider?" << endl;
			        }
			        else {
                        	        qCritical () << "unexpected start: " << flarmid << endl;
                        	        record->setState (FlarmRecord::stateFlying);
                        	        emit actionDetected (flarmid, FlarmHandler::departure);
                        	}
                        	break;
			case FlarmRecord::eventLow:
			        if (record->landingTimer->isActive()) {
        			        qDebug () << "go around2: " << flarmid << endl;
                                	record->setState (FlarmRecord::stateStarting);
                                	record->landingTimer->stop();
                                	if (record->category == Plane::categoryMotorglider)
		        	                emit actionDetected (flarmid, FlarmHandler::goAround);
                                        else
                                                qCritical () << "unexpected goaround of glider?" << endl;
			        }
			        else {
			                record->setState (FlarmRecord::stateStarting);
			                qDebug () << "flat start: " << flarmid << endl;
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
                               	qDebug () << "landing continued: " << flarmid << endl;
                               	record->setState (FlarmRecord::stateOnGround);
                        	if (record->category == Plane::categoryMotorglider)
                        	        record->landingTimer->start(30000);
                                else
                        	        emit actionDetected (flarmid, FlarmHandler::landing);
                        	break;
			case FlarmRecord::eventFly:
			        qDebug () << "go around3: " << flarmid << endl;
                        	record->setState (FlarmRecord::stateFlying);
                        	if (record->category == Plane::categoryMotorglider)
        			        emit actionDetected (flarmid, FlarmHandler::goAround);
                                else
                                        qCritical () << "unexpected goaround of glider?" << endl;
			        break;
                        default:
                                ;
			}
			break;
		case FlarmRecord::stateFlying:
			switch (event) {
			case FlarmRecord::eventGround:
                               	qCritical () << "unexpected landing from high: " << flarmid << endl;
                        	record->setState (FlarmRecord::stateOnGround);
                        	if (record->category == Plane::categoryMotorglider)
                        	        record->landingTimer->start(30000);
                                else
                                	emit actionDetected (flarmid, FlarmHandler::landing);
                        	break;
                        case FlarmRecord::eventLow:
                               	qDebug () << "flying low: " << flarmid << endl;
                                record->setState (FlarmRecord::stateLanding);
                                break;
                        default:
                                ;
			}
			break;
		case FlarmRecord::stateFlyingFar:
			switch (event) {
			case FlarmRecord::eventGround:
                               	qCritical () << "unexpected landing from far: " << flarmid << endl;
                        	record->setState (FlarmRecord::stateOnGround);
                        	if (record->category == Plane::categoryMotorglider)
                        	        record->landingTimer->start(30000);
                                else
                                	emit actionDetected (flarmid, FlarmHandler::landing);
                        	break;
                        case FlarmRecord::eventLow:
                               	qDebug () << "flying low: " << flarmid << endl;
                                record->setState (FlarmRecord::stateLanding);
                                break;
                        case FlarmRecord::eventFly:
                               	qDebug () << "still flying: " << flarmid << endl;
                                record->setState (FlarmRecord::stateFlying);
                                break;
                        default:
                                ;
			}
			break;
		case FlarmRecord::stateStarting:
			switch (event) {
			case FlarmRecord::eventGround: 
                               	qDebug () << "departure aborted: " << flarmid << endl;
                        	record->setState (FlarmRecord::stateOnGround);
                        	emit actionDetected (flarmid, FlarmHandler::landing);
                        	break;
			case FlarmRecord::eventFly:
                               	qDebug () << "departure continued: " << flarmid << endl;
                        	record->setState (FlarmRecord::stateFlying);
			        break;
                        default:
                                ;
			}
			break;
                default:
			switch (event) {
			case FlarmRecord::eventGround: 
                               	qDebug () << "new on ground: " << flarmid << endl;
                        	record->setState (FlarmRecord::stateOnGround);
                        	break;
			case FlarmRecord::eventFly:
                               	qDebug () << "new flying: " << flarmid << endl;
                        	record->setState (FlarmRecord::stateFlying);
			        break;
			case FlarmRecord::eventLow:
                               	qDebug () << "new flying low: " << flarmid << endl;
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
                qDebug () << "keepAliveTimeout: " << flarmid << "; state = " << record->getState() << endl;
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