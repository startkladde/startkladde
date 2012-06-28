#ifndef FLARM_HANDLER_H
#define FLARM_HANDLER_H

#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QMap>
#include <QtCore/QFile>

#include "FlarmRecord.h"
#include "src/db/DbManager.h"
#include "src/model/Plane.h"

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
		const MutableObjectList<FlarmRecord *> *getFlarmRecords () const;

	public slots:
		void lineReceived (const QString &line);

	signals:
		void actionDetected (const QString &id, FlarmRecord::FlightAction);
		void statusChanged ();
		void homePosition (const QPointF&);

	protected:
		int findOrCreateFlarmRecord (const QString &flarmId);

	private:
		FlarmHandler (QObject* parent);
		static FlarmHandler* instance;

		DbManager *dbManager;
		QFile* trace;
		QTextStream* stream;

		int findFlarmRecordByFlarmId (const QString &flarmId);

//		QMap<QString, FlarmRecord *> *regMap;
		// FlarmRecord is a QObject, so we can't store it in a container
		// directly. We have to store a pointer instead.
		MutableObjectList<FlarmRecord *> *flarmRecords;

		QDateTime gpsTime;

	private slots:
		void processPflaaSentence (const PflaaSentence &sentence);
		void processGprmcSentence (const GprmcSentence &sentence);

};

#endif
