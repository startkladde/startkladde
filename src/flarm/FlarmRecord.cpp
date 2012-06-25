/**
 * FlarmRecord.cpp
 * Store data of one flarm device
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
#include <QtDebug>
#include "FlarmRecord.h"
#include "src/i18n/notr.h"

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
                qDebug () << "on ground: " << flarmid;
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
