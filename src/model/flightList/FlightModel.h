/*
 * FlightModel.h
 *
 *  Created on: Aug 30, 2009
 *      Author: Martin Herrmann
 */

#ifndef FLIGHTMODEL_H_
#define FLIGHTMODEL_H_

#include <QVariant>

#include "src/model/objectList/ObjectModel.h"
#include "src/model/objectList/ColumnInfo.h"

class Flight;
namespace Db { namespace Cache { class Cache; } }

/**
 * Unlike the models for Person, Plane and LaunchMethod, this is not part of the
 * respective class because it is much more complex.
 */
class FlightModel: public ObjectModel<Flight>, public ColumnInfo
{
	public:
		FlightModel (Db::Cache::Cache &cache);
		virtual ~FlightModel ();

		virtual int columnCount () const;
		virtual QVariant displayHeaderData (int column) const;
		virtual QVariant data (const Flight &flight, int column, int role=Qt::DisplayRole) const;

		virtual int departButtonColumn () const { return 6; }
		virtual int landButtonColumn () const { return 7; }
		virtual int durationColumn () const { return 8; }

		// ColumnInfo methods
		virtual QString columnName (int columnIndex) const;
		virtual QString sampleText (int columnIndex) const;

	protected:
		virtual QVariant registrationData (const Flight &flight, int role) const;
		virtual QVariant planeTypeData (const Flight &flight, int role) const;
		virtual QVariant pilotData (const Flight &flight, int role) const;
		virtual QVariant copilotData (const Flight &flight, int role) const;
		virtual QVariant launchMethodData (const Flight &flight, int role) const;
		virtual QVariant departureTimeData (const Flight &flight, int role) const;
		virtual QVariant landingTimeData (const Flight &flight, int role) const;
		virtual QVariant durationData (const Flight &flight, int role) const;

	private:
		Db::Cache::Cache &cache;
};

#endif
