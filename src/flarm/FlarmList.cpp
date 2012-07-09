#include "src/flarm/FlarmList.h"

#include <cassert>
#include <iostream>

#include "src/numeric/Velocity.h"
#include "src/nmea/PflaaSentence.h"
#include "src/util/qString.h"
#include "src/nmea/NmeaDecoder.h"

// FIXME caching. Problem is: we can't use a QHash/QMap from flarm ID to index
// because the index may change when items are removed or inserted. What we need
// is something like QPersistentModelIndex, but AbstractObjectList<> does not
// provide that.
// Solutions:
//   * use QAbstract[Item|List]Model, but we need to implement ObjectListModel
//     for that
//   * add persistent index functionality to AbstractObjectList<>, but that
//     would duplicate Qt functionality

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

int FlarmList::findFlarmRecord (const QString &flarmId)
{
	for (int i = 0, n = flarmRecords.size (); i < n; ++i)
		if (flarmRecords.at (i)->flarmId == flarmId) return i;

	return -1;
}

void FlarmList::pflaaSentence (const PflaaSentence &sentence)
{
	// Find the Flarm record for that plane (i. e., Flarm ID)
	int index = findFlarmRecord (sentence.flarmId);

	if (index >= 0)
	{
		// A flarm record was found
		flarmRecords[index]->processPflaaSentence (sentence);
		QAbstractItemModel::dataChanged (QAbstractItemModel::createIndex (index, 0),
				QAbstractItemModel::createIndex (index, 0));
	}
	else
	{
		// There is no Flarm record for that plane yet. Create one, process
		// the sentence and add it to the list.
		FlarmRecord *record = new FlarmRecord (this, sentence.flarmId);

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
		//
		//		connect (record, SIGNAL (actionDetected (const QString &, FlarmRecord::FlightAction)), this, SIGNAL (actionDetected (const QString &, FlarmRecord::FlightAction)));


		// Add it to the list and hash
		int newIndex = flarmRecords.size ();
		QAbstractItemModel::beginInsertRows (QModelIndex (), newIndex, newIndex);
		flarmRecords.append (record);
		QAbstractItemModel::endInsertRows ();

		// FIXME connect the delete signal (there is none yet)
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
	int index = findFlarmRecord (flarmId);
	if (index >= 0)
	{
		QAbstractItemModel::beginRemoveRows (QModelIndex (), index, index);
		flarmRecords.removeAt (index);
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
	// Qt container. This list stores pointer to FlarmRecord.
	// Get rid of AbstractObjectList::getList
	assert (!"Not supported");
}
