#include "src/flarm/FlarmList.h"

#include <cassert>

#include "src/numeric/Velocity.h"
#include "src/flarm/PflaaSentence.h"

// TODO: we have to take care not to let the list and the map get inconsistent.
// We might want to encapsulate that, either in a base class or by placing the
// modification stuff in a controller class.
// A more comprehensive solution would be a separate cache class which listens
// to the signals from the object list.

FlarmList::FlarmList (QObject *parent): AbstractObjectList<FlarmRecord> (parent)
{
}

FlarmList::~FlarmList ()
{
}


// *********************
// ** Data processing **
// *********************

void FlarmList::processPflaaSentence (const PflaaSentence &sentence)
{
	// Find the Flarm record for that plane (i. e., Flarm ID)
	int index=flarmRecordByFlarmId.value (sentence.flarmId, -1);

	if (index>=0)
	{
		// A flarm record was found
		flarmRecords[index]->processPflaaSentence (sentence);
		QAbstractItemModel::dataChanged (QAbstractItemModel::createIndex (index, 0), QAbstractItemModel::createIndex (index, 0));
	}
	else
	{
		// There is no Flarm record for that plane yet. Create one, process
		// the sentence and add it to the list.
		FlarmRecord *record=new FlarmRecord (this, sentence.flarmId);

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
		int newIndex=flarmRecords.size ();
		QAbstractItemModel::beginInsertRows (QModelIndex (), newIndex, newIndex);
		flarmRecords.append (record);
		flarmRecordByFlarmId.insert (sentence.flarmId, newIndex);
		QAbstractItemModel::endInsertRows ();

		// FIXME connect the delete signal (there is none yet)
		// FIXME when the flarm record changes, emit a signal, or only do changes
		// from here
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
