#ifndef FLARM_HANDLER_H
#define FLARM_HANDLER_H

#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QMap>
#include <QtCore/QFile>
#include <QtNetwork/QTcpSocket>

#include "FlarmRecord.h"
#include "src/db/DbManager.h"
#include "src/model/Plane.h"

#include "src/io/dataStream/TcpDataStream.h"

class FlarmHandler: public QObject {

  Q_OBJECT

  public:
    enum FlightAction {departure, landing, goAround};
    static FlarmHandler* getInstance ();
    static QString flightActionToString (FlarmHandler::FlightAction action); 

    ~FlarmHandler ();
    QMap<QString,FlarmRecord*>* getRegMap() {return regMap; }
    void updateList (const Plane&);
    void setDatabase (DbManager*);
    void setEnabled (bool);
    TcpDataStream::State getConnectionState ();
    QDateTime getGPSTime ();
    
  private:
    FlarmHandler (QObject* parent);
    TcpDataStream *dataStream;
    QTimer* refreshTimer;
    QMap<QString, FlarmRecord *> *regMap;
    DbManager *dbManager;
    QFile* trace;
    QTextStream* stream;
    static FlarmHandler* instance;
    QDateTime gpsTime;
    bool enabled;

    double calcLat (const QString& lat, const QString& ns);
    double calcLon (const QString& lon, const QString& ew);

  private slots:
  void processFlarm (const QString &line);
    void keepAliveTimeout ();
    void landingTimeout ();

    signals:
    //    	void connectionStateChanged (TcpDataStream::ConnectionState);
       	void connectionStateChanged (TcpDataStream::State state);

    void actionDetected (const QString& id, FlarmHandler::FlightAction);
    void statusChanged ();
    void homePosition (const QPointF&);
};

#endif
