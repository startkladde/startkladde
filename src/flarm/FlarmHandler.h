#ifndef FLARM_HANDLER_H
#define FLARM_HANDLER_H

#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QMap>
#include <QtCore/QFile>
#include <QtNetwork/QTcpSocket>

#include "src/db/DbManager.h"
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

class FlarmHandler: public QObject {

  Q_OBJECT

  public:
    enum ConnectionState {notConnected, connectedNoData, connectedData};
    enum FlightAction {departure, landing, goAround};
    static FlarmHandler* getInstance ();
    static QString flightActionToString (FlarmHandler::FlightAction action); 

    ~FlarmHandler ();
    QMap<QString,FlarmRecord*>* getRegMap() {return regMap; }
    void updateList (const Plane&);
    void setDatabase (DbManager*);
    ConnectionState getConnectionState ();
    QDateTime getGPSTime ();
    
  private:
    FlarmHandler (QObject* parent);
    QTcpSocket* flarmSocket;
    QTimer* flarmDataTimer;
    QTimer* flarmSocketTimer;
    QTimer* refreshTimer;
    ConnectionState connectionState;
    QMap<QString,FlarmRecord*>* regMap;
    DbManager *dbManager;
    QFile* trace;
    QTextStream* stream;
    static FlarmHandler* instance;
    QDateTime gpsTime;

    void processFlarm (const QString& line);
    uint calcCheckSum(const QString&);
    bool checkCheckSum(const QString&);
    double calcLat (const QString& lat, const QString& ns);
    double calcLon (const QString& lon, const QString& ew);
    void setConnectionState (ConnectionState);

  private slots:
    void initFlarmSocket();
    void dataReceived ();
    void socketException (QAbstractSocket::SocketError socketError);
    void socketStateChanged(QAbstractSocket::SocketState);
    void flarmDataTimeout (); 
    void flarmSocketTimeout ();
    void keepAliveTimeout ();
    void landingTimeout ();

  signals:
    void actionDetected (const QString& id, FlarmHandler::FlightAction);
    void connectionStateChanged (FlarmHandler::ConnectionState);
    void statusChanged ();
    void homePosition (const QPointF&);
};

#endif
