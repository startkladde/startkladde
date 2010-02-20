#ifndef _PlaneLog_h
#define _PlaneLog_h

#include <QAbstractTableModel>
#include <QString>
//#include <QDateTime>
#include <QList>

#include "src/time/Time.h"
#include "src/db/dbId.h"

class DataStorage;
class Flight;

class PlaneLog: public QAbstractTableModel
{
	Q_OBJECT

	protected:
		PlaneLog (QObject *parent=NULL);
		~PlaneLog ();

		class Entry
		{
			public:
				Entry ();
				virtual ~Entry ();

				static Entry create (const Flight *flight, DataStorage &dataStorage);
				static Entry create (const QList<const Flight *> flights, DataStorage &dataStorage);

				QString registration;

				QDate date;
				QString pilotName;
				int minPassengers;
				int maxPassengers;
				QString departureAirfield;
				QString destinationAirfield;
				Time departureTime;
				Time landingTime;
				int numLandings;
				Time operationTime;
				QString comments;

				bool valid;

				virtual QString dateText () const;
				virtual QString numPassengersString () const;
				virtual QString departureTimeText (bool noLetters=false) const;
				virtual QString landingTimeText (bool noLetters=false) const;
				virtual QString operationTimeText () const;
		};

	public:
		static PlaneLog *createNew (db_id planeId, const QList<Flight> &flights, DataStorage &dataStorage);
		static PlaneLog *createNew (const QList<Flight> &flights, DataStorage &dataStorage);

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

	private:
		QList<Entry> entries;
};

#endif
