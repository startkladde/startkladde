#include "FlarmLog.h"

#include <QSet>

#include "src/model/Flight.h"
#include "src/db/cache/Cache.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/util/qDate.h"
#include "src/flarm/FlarmHandler.h"
#include "src/i18n/notr.h"

// ******************
// ** Construction **
// ******************

FlarmLog::FlarmLog (QObject *parent):
	QAbstractTableModel (parent)
{
        connect (FlarmHandler::getInstance(), SIGNAL(statusChanged()), this, SLOT(refresh()));
}

FlarmLog::~FlarmLog ()
{
}

void FlarmLog::refresh () {
	//TODO: this does not work when the number of records in regMap is increased. Why?
	QModelIndex top = createIndex(regMap->count() - 1, 0, 0);
	QModelIndex bottom = createIndex(regMap->count() - 1, 5, 0);
    
	emit dataChanged (top, bottom);
}


// **************
// ** Creation **
// **************

/**
 * @return
 */
FlarmLog *FlarmLog::createNew ()
{
	FlarmLog *result=new FlarmLog ();

	result->regMap = FlarmHandler::getInstance()->getRegMap();

	return result;
}

// *********************************
// ** QAbstractTableModel methods **
// *********************************

int FlarmLog::rowCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return regMap->size ();
}

int FlarmLog::columnCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return 6;
}

QVariant FlarmLog::data (const QModelIndex &index, int role) const
{
	if (index.row () < 0)
		return QVariant();
	
	QString key = regMap->keys() [index.row ()];
	
	const FlarmRecord* record = (*regMap)[key];

	if (role==Qt::DisplayRole)
	{
		switch (index.column ())
		{
			case 0: return record->flarmid;
			case 1: return record->registration;
			case 2: return QString(qnotr("%1")).arg(record->getAlt());
			case 3: return QString(qnotr("%1")).arg(record->getSpeed());
			case 4: return QString(qnotr("%1")).arg(record->getClimb());
			case 5: return record->getStateText ();
			default: assert (false); return QVariant ();
		}
	}
	else
		return QVariant ();
}

QVariant FlarmLog::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role==Qt::DisplayRole)
	{
		if (orientation==Qt::Horizontal)
		{
			switch (section)
			{
				case 0: return tr ("Flarm ID"); break;
				case 1: return tr ("Registration"); break;
				case 2: return tr ("Altitude"); break;
				case 3: return tr ("Speed"); break;
				case 4: return tr ("Vario"); break;
				case 5: return tr ("State"); break;
			}
		}
		else
		{
			return section+1;
		}
	}

	return QVariant ();
}
