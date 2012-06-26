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

class FlarmHandler: public QObject {

		Q_OBJECT

	public:
		static FlarmHandler* getInstance ();
		static QString flightActionToString (FlarmRecord::FlightAction action);

		~FlarmHandler ();
		QMap<QString,FlarmRecord*>* getRegMap() {return regMap; }
		void updateList (const Plane&);
		void setDatabase (DbManager*);
		void setEnabled (bool);
		QDateTime getGPSTime ();

	public slots:
	void lineReceived (const QString &line);

	private:
		FlarmHandler (QObject* parent);
		QTimer* refreshTimer;
		QMap<QString, FlarmRecord *> *regMap;
		DbManager *dbManager;
		QFile* trace;
		QTextStream* stream;
		static FlarmHandler* instance;
		QDateTime gpsTime;
		bool enabled;

		private slots:
		void processPflaaSentence (const PflaaSentence &sentence);
		void processGprmcSentence (const GprmcSentence &sentence);

		void keepAliveTimeout ();
		void landingTimeout ();

		signals:
		void actionDetected (const QString& id, FlarmRecord::FlightAction);
		void statusChanged ();
		void homePosition (const QPointF&);
};

#endif
