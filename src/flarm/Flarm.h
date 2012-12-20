#ifndef FLARM_H_
#define FLARM_H_

#include <QObject>
#include <QDateTime>

#include "src/numeric/GeoPosition.h"

class DbManager;
class FlarmList;
class GpsTracker;
class NmeaDecoder;
class TcpDataStream;

/**
 * A container for all non-GUI Flarm related functionality
 */
class Flarm: public QObject
{
		Q_OBJECT

	public:
		Flarm (QObject *parent, DbManager &dbManager);
		virtual ~Flarm ();

		TcpDataStream *dataStream  () { return _dataStream;  }
		NmeaDecoder   *nmeaDecoder () { return _nmeaDecoder; }
		GpsTracker    *gpsTracker  () { return _gpsTracker;  }
		FlarmList     *flarmList   () { return _flarmList;   }

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

		TcpDataStream *_dataStream;
		NmeaDecoder   *_nmeaDecoder;
		GpsTracker    *_gpsTracker;
		FlarmList     *_flarmList;

		bool _open;
};

#endif