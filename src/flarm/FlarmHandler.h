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

		// Flarm data
		QDateTime getGpsTime ();

		FlarmList &getFlarmList () { return flarmList; }

	public slots:
		void lineReceived (const QString &line);

	signals:
		void actionDetected (const QString &id, FlarmRecord::FlightAction);
		void homePosition (GeoPosition position);

	private:
		FlarmHandler (QObject* parent);
		static FlarmHandler* instance;

		DbManager *dbManager;

		QFile* trace;
		QTextStream* stream;

		QDateTime gpsTime;

		FlarmList flarmList;

	private slots:
		void processPflaaSentence (const PflaaSentence &sentence);
		void processGprmcSentence (const GprmcSentence &sentence);

};

#endif
