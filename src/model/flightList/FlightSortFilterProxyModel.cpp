/*
 * FlightSortFilterProxyModel.cpp
 *
 *  Created on: Sep 3, 2009
 *      Author: deffi
 */

#include "FlightSortFilterProxyModel.h"

#include <cassert>

#include "src/model/objectList/ObjectListModel.h"
#include "src/model/Flight.h"
#include "src/db/DataStorage.h"
#include "src/model/LaunchMethod.h"

FlightSortFilterProxyModel::FlightSortFilterProxyModel (DataStorage &dataStorage, QObject *parent):
	QSortFilterProxyModel (parent),
	dataStorage (dataStorage),
	showPreparedFlights (true),
	hideFinishedFlights (false), alwaysShowExternalFlights (true), alwaysShowErroneousFlights (true),
	customSorting (true)
{
}

FlightSortFilterProxyModel::~FlightSortFilterProxyModel ()
{
}

bool FlightSortFilterProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const
{
	(void)sourceParent;

	// If the model is not an ObjectListModel<Flight>, the filter is not active
	ObjectListModel<Flight> *flightList=dynamic_cast<ObjectListModel<Flight> *> (sourceModel ());
	if (!flightList) return true;

	// Get the flight from the model
	const Flight &flight=flightList->at (sourceRow);

	if (flight.isPrepared ())
	{
		// Prepared flights are hidden if one of these is true:
		//   - showPreparedFlights is false
		//   - the flight is a towflight

		if (!showPreparedFlights) return false;
		if (flight.isTowflight ()) return false;

		return true;
	}
	else
	{
		if (flight.finished ())
		{
			// Finished flights are shown if one of these is true:
			//   - hideFinishedFlights is false
			//   - the flight is external and alwaysShowExternalFlights is true
			//   - the flight is erroneous and alwaysShowExternalFlights is true

			if (!hideFinishedFlights) return true;
			if (alwaysShowExternalFlights && flight.isExternal ()) return true;
			if (alwaysShowErroneousFlights && flight.isErroneous (dataStorage)) return true;

			return false;
		}
		else
		{
			// Flying flights are always shown
			return true;
		}
	}

	// This should not be reached - every leaf in the decision tree above
	// should return.
	assert (false);
	return true;
}

bool FlightSortFilterProxyModel::lessThan (const QModelIndex &left, const QModelIndex &right) const
{
	ObjectListModel<Flight> *flightList=dynamic_cast<ObjectListModel<Flight> *> (sourceModel ());

	// If the model is not an ObjectListModel<Flight>, use the default sorting.
	// The indicies refer to the source model, so we don't remap them.
	if (customSorting && flightList)
		return flightList->at (left) < flightList->at (right);
	else
		return QSortFilterProxyModel::lessThan (left, right);
}

void FlightSortFilterProxyModel::setCustomSorting (bool customSorting)
{
	this->customSorting=customSorting;

	// Invalidate the current sorted order so the model will actually be
	// sorted on the next call to sort, even if it's the same column as before.
	this->invalidate ();
}

void FlightSortFilterProxyModel::sortCustom ()
{
	setCustomSorting (true);

	// The sort column will be ignored for custom sorting
	sort (0, Qt::AscendingOrder);
}
