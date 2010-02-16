/*
 * FlightProxyList.cpp
 *
 *  Created on: Sep 1, 2009
 *      Author: deffi
 */

#include "FlightProxyList.h"

#include <cassert>

#include <QSet>

#include "src/model/Flight.h"
#include "src/db/DataStorage.h"
#include "src/model/LaunchType.h"


FlightProxyList::FlightProxyList (DataStorage &dataStorage, AbstractObjectList<Flight> &sourceModel, QObject *parent):
	AbstractObjectList<Flight> (parent),
	dataStorage (dataStorage),
	sourceModel (sourceModel)
{
	// TODO: if there is no self launch launch type, the flight will be red
	// in the table, but not show an error in the editor

	// Some signals from the source model can be re-emitted without change
#define reemitSignal(signal) do { QObject::connect (&sourceModel, SIGNAL (signal), this, SIGNAL (signal)); } while (0)
	reemitSignal (columnsAboutToBeInserted (const QModelIndex &, int, int));
	reemitSignal (columnsAboutToBeRemoved (const QModelIndex &, int, int));
	reemitSignal (columnsInserted (const QModelIndex &, int, int));
	reemitSignal (columnsRemoved (const QModelIndex &, int, int));
	reemitSignal (headerDataChanged (Qt::Orientation, int, int));
	reemitSignal (layoutAboutToBeChanged ());
	reemitSignal (layoutChanged ());
	reemitSignal (modelAboutToBeReset ());
#undef reemitSignal

#define receiveSignal(signal) do {QObject::connect (&sourceModel, SIGNAL (signal), this, SLOT (sourceModel_ ## signal)); } while (0)
	receiveSignal (dataChanged (const QModelIndex &, const QModelIndex &));
	receiveSignal (modelReset ());
	receiveSignal (rowsAboutToBeInserted (const QModelIndex &, int, int));
	receiveSignal (rowsAboutToBeRemoved (const QModelIndex &, int, int));
	receiveSignal (rowsInserted (const QModelIndex &, int, int));
	receiveSignal (rowsRemoved (const QModelIndex &, int, int));
#undef receiveSignal
}

FlightProxyList::~FlightProxyList ()
{
}


// **************************
// ** Towflight management **
// **************************

// TODO should be in Flight
bool FlightProxyList::isAirtow (const Flight &flight, LaunchType *launchType) const
{
	// No launch type => no airtow
	if (!id_valid (flight.launchType))
		return false;

	try
	{
		LaunchType lt=dataStorage.getObject<LaunchType> (flight.launchType);
		if (launchType) *launchType=lt;
		return lt.is_airtow ();
	}
	catch (DataStorage::NotFoundException)
	{
		// Launch type not found => no airtow
		return false;
	}
}

void FlightProxyList::addTowflightFor (const Flight &flight, const LaunchType &launchType)
{
	// Determine the ID of the towplane
	// TODO code duplication with updateTowflight
	db_id towplaneId=invalid_id;

	if (launchType.towplane_known ())
		towplaneId=dataStorage.getPlaneIdByRegistration (launchType.towplane);
	else
		towplaneId=flight.towplane;

	// Determine the launch type (self launch) of the towplane
	// TODO this should be cached, but the FlightProxyList may be constructed
	// before the dataStorage has the launch types, so we have to receive
	// DEvents and update the self launch ID on launch type changes. We will
	// have to do that anyway to catch changes in the launch types and
	// potentially other things the towflights depend on.
	db_id selfLaunchId=dataStorage.getLaunchTypeByType (sat_self);

	towflights.append (flight.makeTowflight (towplaneId, selfLaunchId));
}

void FlightProxyList::updateTowflight (db_id id, int towflightIndex)
{
	try
	{
		Flight flight=dataStorage.getObject<Flight> (id);
		LaunchType launchType;
		if (isAirtow (flight, &launchType)) // TODO log error if not
		{
			// Determine the ID of the towplane
			// TODO code duplication with addTowflightFor
			db_id towplaneId=invalid_id;

			if (launchType.towplane_known ())
				towplaneId=dataStorage.getPlaneIdByRegistration (launchType.towplane);
			else
				towplaneId=flight.towplane;

			db_id selfLaunchId=dataStorage.getLaunchTypeByType (sat_self);

			towflights.replace (towflightIndex, flight.makeTowflight (towplaneId, selfLaunchId));
		}
	}
	catch (DataStorage::NotFoundException)
	{
		// Do nothing // TODO log error
	}
}

/**
 *
 * @param id
 * @return a flight index
 */
int FlightProxyList::findFlight (db_id id) const
{
	for (int i=0; i<sourceModel.size (); ++i)
		if (sourceModel.at (i).get_id ()==id)
			return i;

	return -1;
}

/**
 *
 * @param id
 * @return a towflight index
 */
int FlightProxyList::findTowflight (db_id id) const
{
	for (int i=0; i<towflights.size (); ++i)
		if (towflights[i].get_id ()==id)
			return i;

	return -1;
}

bool FlightProxyList::modelIndexIsFlight (int index) const
{
	return index<sourceModel.size ();
}

bool FlightProxyList::modelIndexIsTowflight (int index) const
{
	int towflightIndex=modelIndexToTowflightIndex (index);
	return (towflightIndex>=0 && towflightIndex<towflights.size ());
}

int FlightProxyList::towflightIndexToModelIndex (int towflightIndex) const
{
	return towflightIndex+sourceModel.size ();
}

int FlightProxyList::modelIndexToTowflightIndex (int modelIndex) const
{
	return modelIndex-sourceModel.size ();
}

/**
 *
 * @param index
 * @return a model index
 */
int FlightProxyList::findTowref (int index) const
{
	db_id id=at (index).get_id ();

	if (modelIndexIsFlight (index))
	{
		int towref=findTowflight (id);
		if (towref>=0)
			return towflightIndexToModelIndex (towref);
		else
			return -1;
	}
	else if (modelIndexIsTowflight (index))
	{
		int towref=findFlight (id);
		if (towref>=0)
			return flightIndexToModelIndex (towref);
		else
			return -1;
	}
	else
	{
		return -1;
	}
}




// ****************************************
// ** AbstractObjectList<Flight> methods **
// ****************************************

int FlightProxyList::size () const
{
	return sourceModel.size ()+towflights.size ();
}

const Flight &FlightProxyList::at (int index) const
{
	int numFlights=sourceModel.size ();

	if (index<numFlights)
		return sourceModel.at (index);
	else
		return towflights.at (modelIndexToTowflightIndex (index));
}

QList<Flight> FlightProxyList::getList () const
{
	QList<Flight> result;

	return sourceModel.getList () + towflights;
}

// ************************
// ** Source model slots **
// ************************

// TODO: when emitting/reemitting signals, should we generate a new parent index?

void FlightProxyList::sourceModel_dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	// Emit the change signal for the flight
	emit dataChanged (topLeft, bottomRight);

	// For each of the flights, if there is a corresponding towflight, emit
	// a change signal for the towflight
	for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
	{
		const Flight &flight=sourceModel.at (i);
		db_id id=flight.get_id ();

		// Determine the launch type and whether the flight is an airtow
		LaunchType launchType;
		bool flightIsAirtow=isAirtow (sourceModel.at (i), &launchType);

		// We may or may not have a towflight with that ID, regardless of
		// whether the flight is an airtow, because that may have changed.
		int towflightIndex=findTowflight (id);

		// TODO code duplication with removing/adding
		if (flightIsAirtow)
		{
			// Airtow - make sure that there is a towflight

			if (towflightIndex>=0)
			{
				// Towflight already present
				updateTowflight (id, towflightIndex);

				int modelIndex=towflightIndexToModelIndex (towflightIndex);
				// TODO is this the correct parent to use?
				QModelIndex parent=topLeft.parent ();
				emit dataChanged (createIndex (modelIndex, 0), createIndex (modelIndex, columnCount (parent)-1));
			}
			else
			{
				// No towflight yet - add it

				// TODO code duplication with rows_inserted - should be in addTowflightFor
				int towflightIndex=towflights.size ();
				int modelIndex=towflightIndexToModelIndex (towflightIndex);

				beginInsertRows (QModelIndex (), modelIndex, modelIndex);
				addTowflightFor (flight, launchType);
				endInsertRows ();
			}

		}
		else
		{
			// No airtow - make sure that there is no towflight
			if (towflightIndex>=0)
			{
				// We have a towflight for this flight - remove it.
				int modelIndex=towflightIndexToModelIndex (towflightIndex);

				beginRemoveRows (QModelIndex (), modelIndex, modelIndex);
				towflights.removeAt (towflightIndex);
				endRemoveRows ();
			}
		}
	}
}

void FlightProxyList::sourceModel_modelReset ()
{
	towflights.clear ();

	LaunchType launchType;

	const QList<Flight> flights=sourceModel.getList ();
	foreach (const Flight &flight, flights)	// For each flight in the source model
		if (isAirtow (flight, &launchType))	// Is it an airtow?
			addTowflightFor (flight, launchType);	// Add the towflight to the towflight list (don't notify listeners, we'll reset later)

	reset ();
}

void FlightProxyList::sourceModel_rowsAboutToBeInserted (const QModelIndex &parent, int start, int end)
{
	beginInsertRows (parent, start, end);
}

void FlightProxyList::sourceModel_rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
{
	// Flights will be removed. Remove the corresponding towflights, if there
	// are any.
	// We have to remove the towflights in rowsAboutToBeRemoved, because in
	// rowsRemoved the flights are already gone. We may not call endRemoveRows
	// for the flights here, because they are still there.

	// Iterate over the flights. For each flight, if there is a corresponding
	// towflight, remove the towflight.
	for (int i=start; i<=end; ++i)
	{
		// The towflight has the same ID as the flight
		db_id id=sourceModel.at (i).get_id ();

		int towflightIndex=findTowflight (id);
		if (towflightIndex>=0)
		{
			// We have a towflight for this flight - remove it.
			int modelIndex=towflightIndexToModelIndex (towflightIndex);

			beginRemoveRows (parent, modelIndex, modelIndex);
			towflights.removeAt (towflightIndex);
			endRemoveRows ();
		}
	}

	// Begin the removing of the flights after the towflights have been
	// removed, to avoid overlapping removes.
	beginRemoveRows (parent, start, end);
}

void FlightProxyList::sourceModel_rowsInserted (const QModelIndex &parent, int start, int end)
{
	endInsertRows ();

	// Iterate over the inserted flights, adding the corresponding towflights.
	for (int i=start; i<=end; ++i)
	{
		const Flight &flight=sourceModel.at (i);

		LaunchType launchType;
		if (isAirtow (flight, &launchType))
		{
			// TODO code duplication with sourceModel_dataChanged - this should
			// be in addTowflightFor
			int towflightIndex=towflights.size ();
			int modelIndex=towflightIndexToModelIndex (towflightIndex);

			beginInsertRows (parent, modelIndex, modelIndex);
			addTowflightFor (flight, launchType);
			endInsertRows ();
		}
	}
}

void FlightProxyList::sourceModel_rowsRemoved (const QModelIndex &parent, int start, int end)
{
	// We don't have to do anything to the towflight list - corresponding
	// towflights were removed in rowsAboutToBeRemoved.
	(void)parent;
	(void)start;
	(void)end;

	endRemoveRows ();
}
