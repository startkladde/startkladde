#ifndef FLARM_H_
#define FLARM_H_

#include <QDateTime>
#include <QList>
#include <QObject>

#include "src/container/Maybe.h"
#include "src/numeric/GeoPosition.h"
#include "src/io/dataStream/DataStream.h" // For DataStream::State

class DbManager;
class FlarmList;
class GpsTracker;
class NmeaDecoder;
class SerialDataStream;
class TcpDataStream;

/**
 * A container for all non-GUI Flarm related functionality
 */
class Flarm: public QObject
{
		Q_OBJECT

	public:
		struct ConnectionState
		{
			// State
			bool enabled;
			Maybe<DataStream::State>dataStreamState;
		};

	public:
		// Connection type
		enum ConnectionType { noConnection, serialConnection, tcpConnection };
		static QString               ConnectionType_toString   (ConnectionType type);
		static ConnectionType        ConnectionType_fromString (const QString &string, ConnectionType defaultValue);
		static QString               ConnectionType_text       (ConnectionType type);
		static QList<ConnectionType> ConnectionType_list ();

		Flarm (QObject *parent, DbManager &dbManager);
		virtual ~Flarm ();

		DataStream  *dataStream  () { return _p_dataStream;  }
		NmeaDecoder *nmeaDecoder () { return _nmeaDecoder; }
		GpsTracker  *gpsTracker  () { return _gpsTracker;  }
		FlarmList   *flarmList   () { return _flarmList;   }

		bool isDataValid ();

		ConnectionState connectionState ();

		// GPS tracker facade
		GeoPosition getPosition () const;
		QDateTime getGpsTime () const;


	public slots:
		void open  ();
		void close ();
		void setOpen (bool open);

	signals:
		void connectionStateChanged (Flarm::ConnectionState state);

	protected:
		void updateOpen ();

	protected slots:
		void settingsChanged ();
		void dataStream_stateChanged (DataStream::State state);

	private:
		template<class T> T *ensureTypedDataStream ();

		void setDataStream (DataStream *dataStream);

		DbManager &_dbManager;

		//TcpDataStream *_dataStream;
		DataStream  *_p_dataStream;
		NmeaDecoder *_nmeaDecoder;
		GpsTracker  *_gpsTracker;
		FlarmList   *_flarmList;

		bool _open;
};

#endif
