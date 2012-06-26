#include <QtDebug>
#include "FlarmRecord.h"
#include "src/i18n/notr.h"
#include "src/flarm/PflaaSentence.h"

FlarmRecord::FlarmRecord (QObject* parent, const QString& id, flarmState _state)
        : QObject (parent), flarmid (id), state (_state)
{
        alt   = 0;
        speed = 0;
        climb = 0.0;
        last_speed = 0;
        last_alt   = 0;
        last_climb = 0.0;
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
                qDebug () << "on ground" << flarmid;
                break;
        case stateStarting:
                qDebug () << "starting" << flarmid;
                break;
        case stateFlying:
                qDebug () << "flying" << flarmid;
                break;
        case stateFlyingFar:
                qDebug () << "flying far" << flarmid;
                break;
        case stateLanding:
                qDebug () << "landing" << flarmid;
                break;
        default:
                qDebug () << "unknown" << flarmid;
                break;
        }
}

FlarmRecord::flarmState FlarmRecord::getState () const {
        return state;
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

void FlarmRecord::setClimb (double clmb) {
        climb = clmb;
}

double FlarmRecord::getClimb () const {
        return climb;
}

void FlarmRecord::setSpeed (int spd) {
        last_speed = speed;
        speed = spd;
}

int FlarmRecord::getSpeed () const {
        return speed;
}

void FlarmRecord::setAlt (int a) {
        last_alt = alt;
        alt = a;
}

int FlarmRecord::getAlt () const {
        return alt;
}

bool FlarmRecord::isPlausible () const {
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

void FlarmRecord::processPflaaSentence (const PflaaSentence &sentence)
{
	keepAliveTimer->start (5000);
	FlarmRecord::flarmState old_state = getState ();
	FlarmRecord::flarmEvent event;
	// 10 m/s = 36 km/h
	// FIXME store in m/s
	setSpeed ((sentence.groundSpeed * 36) / 10);
	north = sentence.relativeNorth;
	east = sentence.relativeEast;
	setAlt (sentence.relativeVertical);
	setClimb (sentence.climbRate);
	if (!isPlausible ()) return;

	//                          ground speed
	//           | 0-2 m/s | 2-10 m/s | 10-40 m/s | >40 m/s
	// altitude  |---------+----------+-----------+--------
	// >40m      |ground   |fly       |fly        |fly
	// 0-40m     |ground   |ground    |low        |fly

	if (sentence.groundSpeed < 2)
	{
		// Stationary - on ground
		event = eventGround;
	}
	else if (sentence.groundSpeed < 10)
	{
		if (sentence.relativeVertical < 40)
		{
			// Slow with a low altitude - on ground
			event = eventGround;
		}
		else
		{
			// Slow with a high altitude - flying
			event = eventFly;
		}
	}
	// try to avoid low pass
	else if (sentence.groundSpeed < 40)
	{
		if (sentence.relativeVertical < 40)
		{
			//
			event = eventLow;
		}
		else
		{
			event = eventFly;
		}
	}
	else
	{
		event = eventFly;
	}

	switch (old_state)
	{
		case FlarmRecord::stateOnGround:
			switch (event)
			{
				case FlarmRecord::eventFly:
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
				case FlarmRecord::eventLow:
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
				case FlarmRecord::eventGround:
					qDebug () << "landing continued:" << sentence.flarmId;
					setState (FlarmRecord::stateOnGround);
					if (category == Plane::categoryMotorglider)
						landingTimer->start (30000);
					else
						emit actionDetected (sentence.flarmId, landing);
					break;
				case FlarmRecord::eventFly:
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
				case FlarmRecord::eventGround:
					qCritical () << "unexpected landing from high: " << sentence.flarmId;
					setState (FlarmRecord::stateOnGround);
					if (category == Plane::categoryMotorglider)
						landingTimer->start (30000);
					else
						emit actionDetected (sentence.flarmId, landing);
					break;
				case FlarmRecord::eventLow:
					qDebug () << "flying low:" << sentence.flarmId;
					setState (FlarmRecord::stateLanding);
					break;
				default: break;
			}
			break;
		case FlarmRecord::stateFlyingFar:
			switch (event)
			{
				case FlarmRecord::eventGround:
					qCritical () << "unexpected landing from far: " << sentence.flarmId;
					setState (FlarmRecord::stateOnGround);
					if (category == Plane::categoryMotorglider)
						landingTimer->start (30000);
					else
						emit actionDetected (sentence.flarmId, landing);
					break;
				case FlarmRecord::eventLow:
					qDebug () << "flying low:" << sentence.flarmId;
					setState (FlarmRecord::stateLanding);
					break;
				case FlarmRecord::eventFly:
					qDebug () << "still flying:" << sentence.flarmId;
					setState (FlarmRecord::stateFlying);
					break;
				default: break;
			}
			break;
		case FlarmRecord::stateStarting:
			switch (event)
			{
				case FlarmRecord::eventGround:
					qDebug () << "departure aborted:" << sentence.flarmId;
					setState (FlarmRecord::stateOnGround);
					emit actionDetected (sentence.flarmId, landing);
					break;
				case FlarmRecord::eventFly:
					qDebug () << "departure continued:" << sentence.flarmId;
					setState (FlarmRecord::stateFlying);
					break;
				default: break;
			}
			break;
		default:
			switch (event)
			{
				case FlarmRecord::eventGround:
					qDebug () << "new on ground:" << sentence.flarmId;
					setState (FlarmRecord::stateOnGround);
					break;
				case FlarmRecord::eventFly:
					qDebug () << "new flying:" << sentence.flarmId;
					setState (FlarmRecord::stateFlying);
					break;
				case FlarmRecord::eventLow:
					qDebug () << "new flying low:" << sentence.flarmId;
					setState (FlarmRecord::stateLanding);
					break;
				default: break;
			}
			break;
	}
}
