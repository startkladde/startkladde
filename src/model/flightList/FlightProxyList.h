/*
 * FlightProxyList.h
 *
 *  Created on: Sep 1, 2009
 *      Author: Martin Herrmann
 */

#ifndef _FlightProxyList_h
#define _FlightProxyList_h

#include "src/model/Flight.h"
#include "src/model/objectList/AbstractObjectList.h"
#include "src/db/dbId.h"

class DataStorage;
class LaunchMethod;

class FlightProxyList: public AbstractObjectList<Flight>
{
	Q_OBJECT

	public:
		FlightProxyList (DataStorage &dataStorage, AbstractObjectList<Flight> &, QObject *parent=NULL);
		virtual ~FlightProxyList ();

		// AbstractObjectList<Flight> methods
		virtual int size () const;
		virtual const Flight &at (int index) const;
		virtual QList<Flight> getList () const;

		virtual int findTowref (int index) const;

	protected:
		virtual bool isAirtow (const Flight &flight, LaunchMethod *launchMethod) const;
		virtual void addTowflightFor (const Flight &flight, const LaunchMethod &launchMethod);
		virtual void updateTowflight (db_id id, int towflightIndex);

		virtual int findFlight (db_id id) const;
		virtual int findTowflight (db_id id) const;

		virtual bool modelIndexIsFlight (int index) const;
		virtual bool modelIndexIsTowflight (int index) const;

		virtual int flightIndexToModelIndex (int flightIndex) const { return flightIndex; }
		virtual int modelIndexToFlightIndex (int modelIndex) const { return modelIndex; }
		virtual int towflightIndexToModelIndex (int towflightIndex) const;
		virtual int modelIndexToTowflightIndex (int modelIndex) const;

	protected slots:
		// TODO: on layoutChanged, reset this model
		virtual void sourceModel_dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		virtual void sourceModel_modelReset ();
		virtual void sourceModel_rowsAboutToBeInserted (const QModelIndex &parent, int start, int end);
		virtual void sourceModel_rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		virtual void sourceModel_rowsInserted (const QModelIndex &parent, int start, int end);
		virtual void sourceModel_rowsRemoved (const QModelIndex &parent, int start, int end);

	private:
		DataStorage &dataStorage;

		// The model that contains the flights
		AbstractObjectList<Flight> &sourceModel;

		// The towflights added by this proxy
		QList<Flight> towflights;
};

#endif
