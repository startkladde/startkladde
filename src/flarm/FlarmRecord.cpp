#include "FlarmRecord.h"

#include <iostream>

#include <QtDebug>

#include "src/i18n/notr.h"
#include "src/flarm/PflaaSentence.h"
#include "src/numeric/Speed.h"

// ******************
// ** Construction **
// ******************

FlarmRecord::FlarmRecord (QObject *parent, const QString &flarmId): QObject (parent),
	flarmId (flarmId),
	relativeAltitude (0), lastRelativeAltitude (0),
	groundSpeed (0), lastGroundSpeed (0),
	climbRate (0), lastClimbRate (0),
	state (stateUnknown),
	category (Plane::categoryNone)
{
	keepaliveTimer = new QTimer (this);
	landingTimer   = new QTimer (this);
	connect (keepaliveTimer, SIGNAL (timeout ()), this, SLOT (keepaliveTimeout ()));
	connect (landingTimer  , SIGNAL (timeout ()), this, SLOT (landingTimeout   ()));
}

FlarmRecord::~FlarmRecord ()
{
	// The timers are deleted automatically
}

// ****************
// ** Properties **
// ****************


// FIXME when setRegistration and setCategory are called, we must update the
// model views

void FlarmRecord::setRegistration (const QString &registration)
{
	this->registration=registration;
}

void FlarmRecord::setCategory (Plane::Category category)
{
	this->category=category;
}



// **********
// ** Misc **
// **********

void FlarmRecord::setState (flarmState _state)
{
        state = _state;
        switch (state) {
        case stateOnGround:
                qDebug () << "on ground" << flarmId;
                break;
        case stateStarting:
                qDebug () << "starting" << flarmId;
                break;
        case stateFlying:
                qDebug () << "flying" << flarmId;
                break;
        case stateFlyingFar:
                qDebug () << "flying far" << flarmId;
                break;
        case stateLanding:
                qDebug () << "landing" << flarmId;
                break;
        default:
                qDebug () << "unknown" << flarmId;
                break;
        }
}

QString FlarmRecord::getStateText () const {
        switch (state) {
                case FlarmRecord::stateOnGround:
                        return tr("On ground");
                case FlarmRecord::stateStarting:
                        return tr("Starting");
                case FlarmRecord::stateLanding:
                        return tr("Landing");
                case FlarmRecord::stateFlying:
                        return tr("Flying near airfield");
                case FlarmRecord::stateFlyingFar:
                        return tr("Out of range");
                default:
                        return notr ("---");
        }
}

bool FlarmRecord::isPlausible () const {
        if (abs (lastRelativeAltitude - relativeAltitude) > 100)
                return false;
        // speed is in m/s
        if (abs (lastGroundSpeed - groundSpeed) > (100.0/3.6))
                return false;
        if (groundSpeed < 0)
                return false;
        if (groundSpeed > (300.0/3.6))
                return false;
        return true;
}

void FlarmRecord::processPflaaSentence (const PflaaSentence &sentence)
{
	// There is no setter for the values because they may only be changed
	// together, lest they become inconsistent.
	lastRelativeAltitude=relativeAltitude;
	lastGroundSpeed=groundSpeed;
	lastClimbRate=climbRate;

	// FIXME history should be based on time, not samples - samples may get lost
	previousRelativePositions.enqueue (relativePosition);
	for (int i=0; i<previousRelativePositions.size ()-20; ++i)
		previousRelativePositions.dequeue ();

	relativeAltitude = sentence.relativeVertical;
	groundSpeed      = sentence.groundSpeed;
	climbRate        = sentence.climbRate;
	relativePosition = QPointF (sentence.relativeEast, sentence.relativeNorth);



	keepaliveTimer->start (5000);
	FlarmRecord::flarmState old_state = getState ();
	if (!isPlausible ()) return;

	FlightSituation event=getSituation ();

	switch (old_state)
	{
		case FlarmRecord::stateOnGround:
			switch (event)
			{
				case FlarmRecord::flyingSituation:
					// should not happen. Plane was on ground before, now flying?
					// ignore event?
					if (landingTimer->isActive ())
					{
						qCritical () << "unexpected go around1: " << sentence.flarmId;
						setState (FlarmRecord::stateFlying);
						landingTimer->stop ();
						if (category == Plane::categoryMotorglider)
							emit actionDetected (sentence.flarmId, FlarmRecord::goAround);
						else
							qCritical () << "unexpected goaround of glider?";
					}
					else
					{
						qCritical () << "unexpected start: " << sentence.flarmId;
						setState (FlarmRecord::stateFlying);
						emit actionDetected (sentence.flarmId, departure);
					}
					break;
				case FlarmRecord::lowSituation:
					if (landingTimer->isActive ())
					{
						qDebug () << "go around2:" << sentence.flarmId;
						setState (FlarmRecord::stateStarting);
						landingTimer->stop ();
						if (category == Plane::categoryMotorglider)
							emit actionDetected (sentence.flarmId, goAround);
						else
							qCritical () << "unexpected goaround of glider?";
					}
					else
					{
						setState (FlarmRecord::stateStarting);
						qDebug () << "flat start:" << sentence.flarmId;
						emit actionDetected (sentence.flarmId, departure);
					}
					break;
				default: break;
			}
			break;
		case FlarmRecord::stateLanding:
			switch (event)
			{
				case FlarmRecord::groundSituation:
					qDebug () << "landing continued:" << sentence.flarmId;
					setState (FlarmRecord::stateOnGround);
					if (category == Plane::categoryMotorglider)
						landingTimer->start (30000);
					else
						emit actionDetected (sentence.flarmId, landing);
					break;
				case FlarmRecord::flyingSituation:
					qDebug () << "go around3:" << sentence.flarmId;
					setState (FlarmRecord::stateFlying);
					if (category == Plane::categoryMotorglider)
						emit actionDetected (sentence.flarmId, goAround);
					else
						qCritical () << "unexpected goaround of glider?";
					break;
				default: break;
			}
			break;
		case FlarmRecord::stateFlying:
			switch (event)
			{
				case FlarmRecord::groundSituation:
					qCritical () << "unexpected landing from high: " << sentence.flarmId;
					setState (FlarmRecord::stateOnGround);
					if (category == Plane::categoryMotorglider)
						landingTimer->start (30000);
					else
						emit actionDetected (sentence.flarmId, landing);
					break;
				case FlarmRecord::lowSituation:
					qDebug () << "flying low:" << sentence.flarmId;
					setState (FlarmRecord::stateLanding);
					break;
				default: break;
			}
			break;
		case FlarmRecord::stateFlyingFar:
			switch (event)
			{
				case FlarmRecord::groundSituation:
					qCritical () << "unexpected landing from far: " << sentence.flarmId;
					setState (FlarmRecord::stateOnGround);
					if (category == Plane::categoryMotorglider)
						landingTimer->start (30000);
					else
						emit actionDetected (sentence.flarmId, landing);
					break;
				case FlarmRecord::lowSituation:
					qDebug () << "flying low:" << sentence.flarmId;
					setState (FlarmRecord::stateLanding);
					break;
				case FlarmRecord::flyingSituation:
					qDebug () << "still flying:" << sentence.flarmId;
					setState (FlarmRecord::stateFlying);
					break;
				default: break;
			}
			break;
		case FlarmRecord::stateStarting:
			switch (event)
			{
				case FlarmRecord::groundSituation:
					qDebug () << "departure aborted:" << sentence.flarmId;
					setState (FlarmRecord::stateOnGround);
					emit actionDetected (sentence.flarmId, landing);
					break;
				case FlarmRecord::flyingSituation:
					qDebug () << "departure continued:" << sentence.flarmId;
					setState (FlarmRecord::stateFlying);
					break;
				default: break;
			}
			break;
		default:
			switch (event)
			{
				case FlarmRecord::groundSituation:
					qDebug () << "new on ground:" << sentence.flarmId;
					setState (FlarmRecord::stateOnGround);
					break;
				case FlarmRecord::flyingSituation:
					qDebug () << "new flying:" << sentence.flarmId;
					setState (FlarmRecord::stateFlying);
					break;
				case FlarmRecord::lowSituation:
					qDebug () << "new flying low:" << sentence.flarmId;
					setState (FlarmRecord::stateLanding);
					break;
				default: break;
			}
			break;
	}
}

void FlarmRecord::keepaliveTimeout ()
{
	qDebug () << "keepAliveTimeout:" << flarmId << "; state =" << getState ();
	keepaliveTimer->stop ();
	switch (getState ())
	{
		case FlarmRecord::stateLanding:
			qDebug () << "landing by timeout1:" << flarmId;
			setState (FlarmRecord::stateOnGround);
			emit actionDetected (flarmId, FlarmRecord::landing);
			break;
		case FlarmRecord::stateStarting:
			qDebug () << "out of range:" << flarmId;
			//don't change state
			//setState (FlarmRecord::stateFlyingFar);
			break;
		case FlarmRecord::stateFlying:
			qDebug () << "out of range:" << flarmId;
			setState (FlarmRecord::stateFlyingFar);
			break;
		default:
			break;
	}
}

void FlarmRecord::landingTimeout ()
{
	qDebug () << "landingTimeout:" << flarmId << "; state =" << getState ();
	landingTimer->stop ();
	switch (getState ())
	{
		case FlarmRecord::stateOnGround:
			qDebug () << "landing by timeout2:" << flarmId;
			emit actionDetected (flarmId, FlarmRecord::landing);
			break;
		default:
			qCritical () << "landingTimeout in invalid state: " << getState () << "; flarmid = " << flarmId;
			break;
	}
}

QString FlarmRecord::flightActionToString (FlarmRecord::FlightAction action) {
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

FlarmRecord::FlightSituation FlarmRecord::getSituation () const
{
	//                          ground speed
	//           | 0-2 m/s | 2-10 m/s | 10-40 m/s | >40 m/s
	// altitude  |---------+----------+-----------+--------
	// >40m      |ground   |fly       |fly        |fly
	// 0-40m     |ground   |ground    |low        |fly
	// try to avoid low pass

	if (groundSpeed < 2)
	{
		return groundSituation;
	}
	else if (groundSpeed < 10)
	{
		if (relativeAltitude < 40)
			return groundSituation;
		else
			return flyingSituation;
	}
	else if (groundSpeed < 40)
	{
		if (relativeAltitude < 40)
			return lowSituation;
		else
			return flyingSituation;
	}
	else
	{
		return flyingSituation;
	}
}
