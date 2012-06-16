#ifndef FLARM_RECORD_H
#define FLARM_RECORD_H

#include <QtCore/QString>
#include <QtCore/QTimer>
#include "src/model/Plane.h"

class FlarmRecord: public QObject {

  Q_OBJECT
  
  public:
    enum flarmState {stateUnknown, stateOnGround, stateStarting, stateFlying, stateFlyingFar, stateLanding};
    enum flarmEvent {eventGround, eventLow, eventFly};
    FlarmRecord (QObject* parent, const QString& id, flarmState _state);
    void setState (flarmState state);
    flarmState getState () const;
    QString getStateText () const;
    void setClimb (double);
    double getClimb () const;
    void setSpeed (int);
    int getSpeed () const;
    void setAlt (int);
    int getAlt () const;
    bool isPlausible () const;
    QString flarmid;
    QString registration;
    QString frequence;
    Plane::Category category;
    int north;
    int east;
    QTimer* keepAliveTimer;
    QTimer* landingTimer;
  private:
    flarmState state;
    int alt, last_alt;
    int speed, last_speed;
    double climb, last_climb;
  signals:
    void keepAliveTimeout();
    void landingTimeout();
};

#endif
