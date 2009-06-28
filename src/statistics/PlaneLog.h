#ifndef _PlaneLog_h
#define _PlaneLog_h

#include <QString>
#include <QDateTime>

#include "src/db/Database.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/time/Time.h"

class PlaneLogEntry
{
	public:
		PlaneLogEntry ();

		QString club;
		QString registration;
		QString flugzeug_typ;
		QDate datum;
		QString name;
		int insassen;
		QString ort_von;
		QString ort_nach;
		Time zeit_start;
		Time zeit_landung;
		int anzahl_landungen;
		Time betriebsdauer;
		QString bemerkungen;
		bool invalid;

		QString insassen_string () const;
		QString datum_string () const;
		QString zeit_start_string (bool no_letters=false) const;
		QString zeit_landung_string (bool no_letters=false) const;
		QString betriebsdauer_string () const;
		QString anzahl_landungen_string () const;
};

bool makePlaneLogEntry (PlaneLogEntry *bb_entry, Database *db, QList<Flight *> &flights, Plane &fz, QDate date);
void makePlaneLogPlane (QList<PlaneLogEntry *> &bb, Database *db, QDate date, Plane &fz, QList<Flight *> &flights);
void makePlaneLogDay (QList<PlaneLogEntry *> &bb, Database *db, QDate date, QList<Plane *> planes, QList<Flight *> flights, QString *club=NULL);
void makePlaneLogDay (QList<PlaneLogEntry *> &bb, Database *db, QDate date);

#endif

