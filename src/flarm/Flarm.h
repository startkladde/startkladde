#ifndef FLARM_H_
#define FLARM_H_

#include <QDateTime>
#include <QList>
#include <QObject>

#include "src/numeric/GeoPosition.h"

class DataStream;
class DbManager;
class FlarmList;
class GpsTracker;
class NmeaDecoder;

/**
 * A container for all non-GUI Flarm related functionality
 */
class Flarm: public QObject
{
		Q_OBJECT

	public:
		// Connection type
		enum ConnectionType { noConnection, serialConnection, tcpConnection };
		static QString        ConnectionType_toString   (ConnectionType type);
		static ConnectionType ConnectionType_fromString (const QString &string, ConnectionType defaultValue);
		static QString        ConnectionType_text       (ConnectionType type);
		static QList<ConnectionType> ConnectionType_list ();

		Flarm (QObject *parent, DbManager &dbManager);
		virtual ~Flarm ();

		DataStream  *dataStream  () { return _dataStream;  }
		NmeaDecoder *nmeaDecoder () { return _nmeaDecoder; }
		GpsTracker  *gpsTracker  () { return _gpsTracker;  }
		FlarmList   *flarmList   () { return _flarmList;   }

		bool isDataValid ();

		// GPS tracker facade
		GeoPosition getPosition () const;
		QDateTime getGpsTime () const;



	public slots:
		void open  ();
		void close ();
		void setOpen (bool open);

	protected:
		void updateOpen ();

	protected slots:
		void settingsChanged ();

	private:
		DbManager &_dbManager;

		//TcpDataStream *_dataStream;
		DataStream  *_dataStream;
		NmeaDecoder *_nmeaDecoder;
		GpsTracker  *_gpsTracker;
		FlarmList   *_flarmList;

		bool _open;
};

#endif
