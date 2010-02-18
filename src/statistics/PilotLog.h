#ifndef _PilotLog_h
#define _PilotLog_h

#include <QString>
#include <QDateTime>
#include <QAbstractTableModel>
#include <QList>

#include "src/time/Time.h"
#include "src/db/dbId.h"

class DataStorage;
class Flight;

/*
 * TODO: display guest as "(Guest)"
 */
class PilotLog: public QAbstractTableModel
{
	Q_OBJECT

	protected:
		PilotLog (QObject *parent=NULL);
		~PilotLog ();

		class Entry
		{
			public:
				Entry ();
				virtual ~Entry ();

				static Entry create (const Flight *flight, DataStorage &dataStorage);

				QDate date;
				QString planeType;
				QString planeRegistration;
				QString pilot;
				QString copilot;
				QString launchMethod;
				QString departureAirfield;
				QString destinationAirfield;
				Time departureTime;
				Time landingTime;
				Time flightDuration;
				QString comments;

				bool valid;

				virtual QString dateText () const;
				virtual QString departureTimeText (bool noLetters=false) const;
				virtual QString landingTimeText (bool noLetters=false) const;
				virtual QString flightDurationText () const;
		};

	public:
		enum FlightInstructorMode { flightInstructorNone, flightInstructorStrict, flightInstructorLoose };

		static PilotLog *createNew (db_id personId, const QList<Flight> &flights, DataStorage &dataStorage, FlightInstructorMode mode=flightInstructorNone);
		static PilotLog *createNew (const QList<Flight> &flights, DataStorage &dataStorage, FlightInstructorMode mode=flightInstructorNone);

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

	private:
		QList<Entry> entries;
};

#endif
