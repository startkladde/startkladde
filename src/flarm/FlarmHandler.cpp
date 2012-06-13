/**
 * flarm_handler.cpp
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

#include "FlarmHandler.h"
#include "src/model/Plane.h"
//#include "FlarmNetDb.h"

FlarmRecord::FlarmRecord (QObject* parent, const QString& _id, flarmState _state)
        : QObject (parent), id (_id), state (_state)
{
        alt   = 0;
        speed = 0;
        climb = 0.0;
        last_speed = 0;
        last_alt   = 0;
        last_climb = 0.0;
        //intervals = 0;
        north = 0;
        east  = 0;
        category = Plane::categoryNone;
        keepAliveTimer = new QTimer (this);
        landingTimer = new QTimer (this);
        connect (keepAliveTimer, SIGNAL(timeout()), this, SIGNAL(keepAliveTimeout()));
        connect (landingTimer, SIGNAL(timeout()), this, SIGNAL(landingTimeout()));
}

void FlarmRecord::setState (flarmState _state)
{
        state = _state;
        switch (state) {
        case stateOnGround:
                qDebug () << "on ground: " << id << endl;
                break;
        case stateStarting:
                qDebug () << "starting" << id << endl;
                break;
        case stateFlying:
                qDebug () << "flying" << id << endl;
                break;
        case stateFlyingFar:
                qDebug () << "flying far" << id << endl;
                break;
        case stateLanding:
                qDebug () << "landing" << id << endl;
                break;
        default:
                qDebug () << "unknown" << id << endl;
                break;
        }
}

FlarmRecord::flarmState FlarmRecord::getState ()
{
        return state;
}

void FlarmRecord::setClimb (double clmb) {
        climb = clmb;
        //interval ++;
}

double FlarmRecord::getClimb () {
        //double result = climb / intervals;
        //climb = 0.0;
        //intervals = 0;
        //return restult;
        return climb;
}

void FlarmRecord::setSpeed (int spd) {
        last_speed = speed;
        speed = spd;
}

int FlarmRecord::getSpeed () {
        return speed;
}

void FlarmRecord::setAlt (int a) {
        last_alt = alt;
        alt = a;
}

int FlarmRecord::getAlt () {
        return alt;
}

bool FlarmRecord::isPlausible () {
        if (abs (last_alt - alt) > 100) 
                return false;
        // speed is in km/h
        if (abs (last_speed - speed) > 100)
                return false;
        if (speed < 0)
                return false;
        if (speed > 300)
                return false;
        return true;
}

FlarmHandler* FlarmHandler::instance = NULL;

FlarmHandler::FlarmHandler (QObject* parent) 
  :QObject (parent)
{
        regMap = NULL;
        flarmSocket = new QTcpSocket (this);
        // force to emit signal
        connectionState = connectedNoData;
        setConnectionState (notConnected);
        flarmDataTimer = new QTimer (this);
        flarmSocketTimer = new QTimer (this);
        refreshTimer = new QTimer (this);
        connect (flarmDataTimer, SIGNAL(timeout()), this, SLOT(flarmDataTimeout()));
        connect (flarmSocketTimer, SIGNAL(timeout()), this, SLOT(flarmSocketTimeout()));
        connect (refreshTimer, SIGNAL(timeout()), this, SIGNAL(statusChanged()));
        connect (flarmSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
        connect (flarmSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketException(QAbstractSocket::SocketError)));
        connect (flarmSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));

        regMap = new QMultiMap <QString, FlarmRecord*> ();

	// initialize Flarm TcpPort
	QTimer::singleShot (0, this, SLOT(initFlarmSocket()));
	refreshTimer->start (5000);

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

FlarmHandler* FlarmHandler::getInstance () {
        if (instance == NULL)
                instance = new FlarmHandler (NULL);
        return instance;
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

/*
void FlarmHandler::updateList (const sk_flugzeug& plane) {
        if (!plane.flarm_id.empty()) {
                FlarmRecord* record = regMap->value (std2q(plane.flarm_id));
                if (record)
                        record->reg = std2q(plane.registration);
        }
}
*/

/*
bool FlarmHandler::connected () {
        if (flarmSocket)
                return flarmSocket->state() == QAbstractSocket::ConnectedState;
        else
                return false;
}

bool FlarmHandler::active() {
	return flarmStatus;
}
*/

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

void FlarmHandler::initFlarmSocket () {
	// will be set on first sentence
	//flarmStatus = false;
	setConnectionState (notConnected);
	
	//qDebug () << "FlarmHandler::initFlarmSocket: " << endl;

	if (flarmSocket->state() == QAbstractSocket::ConnectedState) {
		qDebug () << "FlarmSocket is connected" << endl;
		// stop the timer. As long as no exception, no timer.
		flarmSocketTimer->stop ();
		return;
	}
 	else if (flarmSocket->state() == QAbstractSocket::ConnectingState) {
 	        // this does not seem to happen
		qDebug () << "FlarmSocket is connecting" << endl;
		//flarmSocketTimer->stop ();
		flarmSocket->abort ();
		return;
 	}
 	else
 	        //qDebug() << "state: " << flarmSocket->state() << endl;
	
        //qDebug () << "try to connect flarm socket" << endl;
	flarmSocket->connectToHost ("localhost", 4711, QIODevice::ReadOnly);

	flarmDataTimer->start (2000);
	flarmSocketTimer->start (5000);
}

void FlarmHandler::dataReceived () {
	//qDebug () << "dataReceived" << endl;
	//flarmStatus = true;
	setConnectionState (connectedData);
	flarmDataTimer->start(2000);
	while (flarmSocket->canReadLine()) {
		QString line = flarmSocket->readLine ();
		// qDebug () << "line: " << line << endl;
		processFlarm (line);
	}
}

void FlarmHandler::socketException (QAbstractSocket::SocketError socketError) {
	//qDebug () << "socketException: " << socketError << endl;
	flarmSocket->abort();
	//flarmStatus = false;
	//setConnectionState (notConnected);
	flarmSocketTimer->start (5000);
}

void FlarmHandler::flarmDataTimeout () {
	//qDebug () << "FlarmHandler::flarmDataTimeout" << endl;
	//flarmStatus = false;
	setConnectionState (connectedNoData);
	// if no data, most likely the socket is down. Give it enough time to recover     
	flarmDataTimer->start(6000);
}

void FlarmHandler::flarmSocketTimeout () {
	//qDebug () << "FlarmHandler::flarmSocketTimeout" << endl;
	//flarmStatus = false;        
	// do not start timer; initFlarmSocket will take care
	//flarmSocketTimer->start(5000);
	initFlarmSocket ();
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

/** This function checks if the checksum in the sentence matches the sentence. It retuns true if it matches, and false otherwise. */
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
                                record->reg = plane->registration;
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
		        	                emit goaroundDetected (flarmid);
                                        else
                                                qCritical () << "unexpected goaround of glider?" << endl;
			        }
			        else {
                        	        qCritical () << "unexpected start: " << flarmid << endl;
                        	        record->setState (FlarmRecord::stateFlying);
                        	        emit departureDetected (flarmid);
                        	}
                        	break;
			case FlarmRecord::eventLow:
			        if (record->landingTimer->isActive()) {
        			        qDebug () << "go around2: " << flarmid << endl;
                                	record->setState (FlarmRecord::stateStarting);
                                	record->landingTimer->stop();
                                	if (record->category == Plane::categoryMotorglider)
		        	                emit goaroundDetected (flarmid);
                                        else
                                                qCritical () << "unexpected goaround of glider?" << endl;
			        }
			        else {
			                record->setState (FlarmRecord::stateStarting);
			                qDebug () << "flat start: " << flarmid << endl;
			                emit departureDetected (flarmid);
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
                        	        emit landingDetected (flarmid);
                        	break;
			case FlarmRecord::eventFly:
			        qDebug () << "go around3: " << flarmid << endl;
                        	record->setState (FlarmRecord::stateFlying);
                        	if (record->category == Plane::categoryMotorglider)
        			        emit goaroundDetected (flarmid);
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
                                	emit landingDetected (flarmid);
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
                                	emit landingDetected (flarmid);
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
                        	emit landingDetected (flarmid);
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
	        QDateTime datetime (date.addYears(100), time, Qt::UTC);
	        //qDebug () << "datetime: " << datetime.toString ("hh:mm:ss dd.MM.yy");
	        emit updateDateTime (datetime);
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
                QString id = record->id;
                qDebug () << "keepAliveTimeout: " << id << "; state = " << record->getState() << endl;
                record->keepAliveTimer->stop();
                switch (record->getState()) {
                case FlarmRecord::stateLanding:
                        qDebug () << "landing by timeout1: " << id << endl;
                        record->setState (FlarmRecord::stateOnGround);
                        emit landingDetected (id);
                        break;
                case FlarmRecord::stateStarting:
                        qDebug () << "out of range: " << id << endl;
                        //don't change state
                        //record->setState (FlarmRecord::stateFlyingFar);
                        break;
                case FlarmRecord::stateFlying:
                        qDebug () << "out of range: " << id << endl;
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
                QString id = record->id;
                qDebug () << "landingTimeout: " << id << "; state = " << record->getState() << endl;
                record->landingTimer->stop();
                switch (record->getState()) {
                case FlarmRecord::stateOnGround:
                        qDebug () << "landing by timeout2: " << id << endl;
                        emit landingDetected (id);
                        break;
                default:
                        qCritical () << "landingTimeout in invalid state: " << record->getState() << "; id = " << id << endl;
                }
        }
}
