#ifndef PLANELOG_H_
#define PLANELOG_H_

#include <QAbstractTableModel>
#include <QString>
//#include <QDateTime>
#include <QList>
#include <QDate>

#include "src/db/dbId.h"

class Cache;
class Flight;

class PlaneLog: public QAbstractTableModel
{
	protected:
		PlaneLog (QObject *parent=NULL);
		~PlaneLog ();

		class Entry
		{
			public:
				Entry ();
				virtual ~Entry ();

				static Entry create (const Flight *flight, Cache &cache);
				static Entry create (const QList<const Flight *> flights, Cache &cache);

				QString registration;
				QString type;

				QDate date;
				QString pilotName;
				int minPassengers;
				int maxPassengers;
				QString departureLocation;
				QString landingLocation;
				QDateTime departureTime;
				QDateTime  landingTime;
				int numLandings;
				QTime operationTime;
				QString comments;

				bool valid;

				virtual QString dateText () const;
				virtual QString numPassengersString () const;
				virtual QString departureTimeText () const;
				virtual QString landingTimeText () const;
				virtual QString operationTimeText () const;
		};

	public:
		static PlaneLog *createNew (dbId planeId, const QList<Flight> &flights, Cache &cache);
		static PlaneLog *createNew (const QList<Flight> &flights, Cache &cache);

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

	private:
		QList<Entry> entries;
};

#endif
