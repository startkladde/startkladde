#ifndef FLARM_HANDLER_H
#define FLARM_HANDLER_H

#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QMap>
#include <QtCore/QFile>
#include <QTime>

#include "FlarmRecord.h"
#include "src/model/Plane.h"
#include "src/numeric/GeoPosition.h"
#include "src/flarm/FlarmList.h"
#include "src/nmea/GpsTracker.h"

class PflaaSentence;
class GprmcSentence;
class NmeaDecoder;
template<class T> class MutableObjectList;

class FlarmHandler: public QObject
{
	Q_OBJECT

	public:
		// Construction/singleton
		static FlarmHandler* getInstance ();
		~FlarmHandler ();

		// Properties
		void setNmeaDecoder (NmeaDecoder *nmeaDecoder);

	signals:
		void actionDetected (const QString &id, FlarmRecord::FlightAction);

	private:
		FlarmHandler (QObject* parent);
		static FlarmHandler* instance;

		QFile* trace;
		QTextStream* stream;

};

#endif
