#ifndef FLARM_HANDLER_H
#define FLARM_HANDLER_H

#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QMap>
#include <QtCore/QFile>

#include "FlarmRecord.h"
#include "src/db/DbManager.h"
#include "src/model/Plane.h"
#include "src/numeric/GeoPosition.h"
#include "src/flarm/FlarmList.h"

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
		void setDatabase (DbManager*);
		void setNmeaDecoder (NmeaDecoder *nmeaDecoder);

		// Flarm data
		QDateTime getGpsTime ();

		FlarmList &getFlarmList () { return flarmList; }

	signals:
		void actionDetected (const QString &id, FlarmRecord::FlightAction);
		void homePosition (GeoPosition position);

	private:
		FlarmHandler (QObject* parent);
		static FlarmHandler* instance;

		DbManager *dbManager;
		NmeaDecoder *nmeaDecoder;

		QFile* trace;
		QTextStream* stream;

		QDateTime gpsTime;

		FlarmList flarmList;

	private slots:
		void pflaaSentence (const PflaaSentence &sentence);
		void gprmcSentence (const GprmcSentence &sentence);

};

#endif
