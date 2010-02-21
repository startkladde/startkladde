#include "LaunchMethodStatistics.h"

#include "src/model/LaunchMethod.h"
#include "src/model/Flight.h"
#include "src/db/DataStorage.h"

// ************************
// ** Entry construction **
// ************************

LaunchMethodStatistics::Entry::Entry ()
{
}

LaunchMethodStatistics::Entry::~Entry ()
{
}

// ******************
// ** Construction **
// ******************

LaunchMethodStatistics::LaunchMethodStatistics (QObject *parent):
	QAbstractTableModel (parent)
{
}

LaunchMethodStatistics::~LaunchMethodStatistics ()
{
}

// **************
// ** Creation **
// **************

LaunchMethodStatistics *LaunchMethodStatistics::createNew (const QList<Flight> &flights, DataStorage &dataStorage)
{
	QMap<dbId, int> map;

	foreach (const Flight &flight, flights)
	{
		if (flight.happened ())
		{
			// Non-existing values are initialized to 0
			++map[flight.launchMethodId];
		}
	}

	// Make a list of launch methods and sort it
	QList<LaunchMethod> launchMethods;
	foreach (const dbId &id, map.keys ())
	{
		try
		{
			launchMethods.append (dataStorage.getObject<LaunchMethod> (id));
		}
		catch (...)
		{
			// TODO log error
		}
	}
	qSort (launchMethods.begin (), launchMethods.end (), LaunchMethod::nameLessThan);

	LaunchMethodStatistics *result=new LaunchMethodStatistics ();
	foreach (const LaunchMethod &launchMethod, launchMethods)
	{
		Entry entry;
		entry.name=launchMethod.name;
		entry.num=map[launchMethod.getId ()];
		result->entries.append (entry);
	}

	return result;
}

// *********************************
// ** QAbstractTableModel methods **
// *********************************

int LaunchMethodStatistics::rowCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return entries.size ();
}

int LaunchMethodStatistics::columnCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return 2;
}

QVariant LaunchMethodStatistics::data (const QModelIndex &index, int role) const
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

QVariant LaunchMethodStatistics::headerData (int section, Qt::Orientation orientation, int role) const
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
