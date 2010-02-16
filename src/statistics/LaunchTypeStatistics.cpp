/*
 * LaunchTypeStatistics.cpp
 *
 *  Created on: Aug 18, 2009
 *      Author: mherrman
 */

#include "LaunchTypeStatistics.h"

#include "src/model/LaunchType.h"
#include "src/db/DataStorage.h"

// ************************
// ** Entry construction **
// ************************

LaunchTypeStatistics::Entry::Entry ()
{
}

LaunchTypeStatistics::Entry::~Entry ()
{
}

// ******************
// ** Construction **
// ******************

LaunchTypeStatistics::LaunchTypeStatistics (QObject *parent):
	QAbstractTableModel (parent)
{
}

LaunchTypeStatistics::~LaunchTypeStatistics ()
{
}

// **************
// ** Creation **
// **************

LaunchTypeStatistics *LaunchTypeStatistics::createNew (const QList<Flight> &flights, DataStorage &dataStorage)
{
	QMap<db_id, int> map;

	foreach (const Flight &flight, flights)
	{
		if (flight.happened ())
		{
			// Non-existing values are initialized to 0
			++map[flight.launchType];
		}
	}

	// Make a list of launch types and sort it
	QList<LaunchType> launchTypes;
	foreach (const db_id &id, map.keys ())
	{
		try
		{
			launchTypes.append (dataStorage.getObject<LaunchType> (id));
		}
		catch (...)
		{
			// TODO log error
		}
	}
	qSort (launchTypes.begin (), launchTypes.end (), LaunchType::nameLessThan);

	LaunchTypeStatistics *result=new LaunchTypeStatistics ();
	foreach (const LaunchType &launchType, launchTypes)
	{
		Entry entry;
		entry.name=launchType.description;
		entry.num=map[launchType.get_id ()];
		result->entries.append (entry);
	}

	return result;
}

// *********************************
// ** QAbstractTableModel methods **
// *********************************

int LaunchTypeStatistics::rowCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return entries.size ();
}

int LaunchTypeStatistics::columnCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return 2;
}

QVariant LaunchTypeStatistics::data (const QModelIndex &index, int role) const
{
	const Entry &entry=entries[index.row ()];

	if (role==Qt::DisplayRole)
	{
		switch (index.column ())
		{
			case 0: return entry.name;
			case 1: return entry.num;
			default: assert (false); return QVariant ();
		}
	}
	else
		return QVariant ();
}

QVariant LaunchTypeStatistics::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role==Qt::DisplayRole)
	{
		if (orientation==Qt::Horizontal)
		{
			switch (section)
			{
				case 0: return "Startart"; break;
				case 1: return "Anzahl Starts"; break;
			}
		}
		else
		{
			return section+1;
		}
	}

	return QVariant ();
}
