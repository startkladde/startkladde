#include "src/flarm/FlarmList.h"

#include <cassert>
#include <iostream>

#include "src/numeric/Velocity.h"
#include "src/nmea/PflaaSentence.h"
#include "src/util/qString.h"
#include "src/nmea/NmeaDecoder.h"

// TODO: we have to take care not to let the list and the map get inconsistent.
// We might want to encapsulate that, either in a base class or by placing the
// modification stuff in a controller class.
// A more comprehensive solution would be a separate cache class which listens
// to the signals from the object list.

FlarmList::FlarmList (QObject *parent) :
	AbstractObjectList<FlarmRecord> (parent)
{
}

FlarmList::~FlarmList ()
{
}


// ******************
// ** NMEA decoder **
// ******************

void FlarmList::setNmeaDecoder (NmeaDecoder *nmeaDecoder)
{
	if (this->nmeaDecoder)
	{
		this->nmeaDecoder->disconnect (this);
	}

	this->nmeaDecoder=nmeaDecoder;

	if (this->nmeaDecoder)
	{
		connect (this->nmeaDecoder, SIGNAL (pflaaSentence (const PflaaSentence &)), this, SLOT (pflaaSentence (const PflaaSentence &)));
	}
}

// *********************
// ** Data processing **
// *********************

void FlarmList::pflaaSentence (const PflaaSentence &sentence)
{
	// Find the Flarm record for that plane (i. e., Flarm ID)
	QModelIndex recordIndex = byFlarmId.value (sentence.flarmId);

	if (recordIndex.isValid ())
	{
		// A flarm record was found
		flarmRecords[recordIndex.row ()]->processPflaaSentence (sentence);
		QAbstractItemModel::dataChanged (recordIndex, recordIndex);
	}
	else
	{
		// There is no Flarm record for that plane yet. Create one, process
		// the sentence and add it to the list.
		FlarmRecord *record = new FlarmRecord (this, sentence.flarmId);

		connect (record, SIGNAL (departureDetected (const QString &)), this, SIGNAL (departureDetected (const QString &)));
		connect (record, SIGNAL (landingDetected   (const QString &)), this, SIGNAL (landingDetected   (const QString &)));
		connect (record, SIGNAL (goAroundDetected  (const QString &)), this, SIGNAL (goAroundDetected  (const QString &)));
		connect (record, SIGNAL (remove (const QString &)), this, SLOT (removeFlarmRecord (const QString &)));

		record->processPflaaSentence (sentence);

		// FIXME
		//		// try get info from flarmnet database
		//		/*
		//		 FlarmNetRecord* flarmnet_record = FlarmNetDb::getInstance()->getData (flarmid);
		//		 if (flarmnet_record) {
		//		 record->freq = flarmnet_record->freq;
		//		 record->reg  = flarmnet_record->registration;
		//		 }
		//		 */
		//
		//		// Try to get the registration from own database
		//		Plane *plane;
		//		dbId planeId = dbManager->getCache ().getPlaneIdByFlarmId (flarmId);
		//		if (idValid (planeId))
		//		{
		//			plane = dbManager->getCache ().getNewObject<Plane> (planeId);
		//			record->setRegistration (plane->registration);
		//			record->setCategory (plane->category);
		//		}

		// FIXME separate method for adding/removing, updating the cache
		// Add it to the list and hash
		int newRow = flarmRecords.size ();

		QAbstractItemModel::beginInsertRows (QModelIndex (), newRow, newRow);

		// Add to list
		flarmRecords.append (record);

		// Add to cache
		Q_ASSERT (!byFlarmId.contains (sentence.flarmId));
		byFlarmId.insert (sentence.flarmId, QPersistentModelIndex (index (newRow)));

		QAbstractItemModel::endInsertRows ();

		// FIXME when the flarm record changes, emit a signal, or only do changes
		// from here
	}
}

// **********************
// ** Controller slots **
// **********************

// It's not a proper controller in the MVC sense, it's integrated with the
// model. We may want to change this some time.

void FlarmList::removeFlarmRecord (const QString &flarmId)
{
	QModelIndex index = byFlarmId.value (flarmId);
	if (index.isValid ())
	{
		int row=index.row ();

		QAbstractItemModel::beginRemoveRows (QModelIndex (), row, row);

		// Remove from list
		flarmRecords.removeAt (row);

		// Remove from cache
		Q_ASSERT (byFlarmId.contains (flarmId));
		byFlarmId.remove (flarmId);

		QAbstractItemModel::endRemoveRows ();
	}
}

// ********************************
// ** AbstractObjectList methods **
// ********************************

int FlarmList::size () const
{
	return flarmRecords.size ();
}

const FlarmRecord &FlarmList::at (int index) const
{
	return *flarmRecords.at (index);
}

QList<FlarmRecord> FlarmList::getList () const
{
	// FIXME we cannot implement this because we cannot store FlarmRecord in a
	// Qt container. This list stores pointer to FlarmRecord. Get rid of
	// AbstractObjectList::getList
	assert (!"Not supported");
}
