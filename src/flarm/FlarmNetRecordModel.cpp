#include <src/flarm/FlarmNetRecordModel.h>

#include <iostream>

#include <QApplication>

#include "src/flarm/FlarmNetRecord.h"
#include "src/i18n/notr.h"
#include "src/util/qString.h"
#include "src/numeric/Velocity.h"

FlarmNetRecordModel::FlarmNetRecordModel ()
{
}

FlarmNetRecordModel::~FlarmNetRecordModel ()
{
}

int FlarmNetRecordModel::columnCount () const
{
	return 6;
}

QVariant FlarmNetRecordModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return qApp->translate ("FlarmNetRecord::DefaultObjectModel", "Flarm ID");
		case 1: return qApp->translate ("FlarmNetRecord::DefaultObjectModel", "Registration");
		case 2: return qApp->translate ("FlarmNetRecord::DefaultObjectModel", "Callsign");
		case 3: return qApp->translate ("FlarmNetRecord::DefaultObjectModel", "Owner");
		case 4: return qApp->translate ("FlarmNetRecord::DefaultObjectModel", "Model");
		case 5: return qApp->translate ("FlarmNetRecord::DefaultObjectModel", "Frequency");
	}

	return QVariant ();
}


QVariant FlarmNetRecordModel::data (FlarmNetRecord * const &flarmRecord, int column, int role) const
{
	if (role==Qt::DisplayRole)
		return displayData (flarmRecord, column);
	else if (role==Qt::TextAlignmentRole)
		return alignmentData (flarmRecord, column);
	else
		return QVariant ();
}

QVariant FlarmNetRecordModel::displayData (FlarmNetRecord* const &object, int column) const
{
	switch (column)
	{
		case 0: return object->getFlarmId();
		case 1: return object->getRegistration();
		case 2: return object->getCallsign();
		case 3: return object->getOwner();
		case 4: return object->getType();
		case 5: return object->getFrequency();
	}

	return QVariant ();
}

QVariant FlarmNetRecordModel::alignmentData (FlarmNetRecord * const &flarmRecord, int column) const
{
	Q_UNUSED (flarmRecord);

	switch (column)
	{
		case 0: return Qt::AlignLeft;
		case 1: return Qt::AlignLeft;
		case 2: return Qt::AlignLeft;
		case 3: return Qt::AlignLeft;
		case 4: return Qt::AlignLeft;
		case 5: return Qt::AlignLeft;
	}

	return QVariant ();
}
