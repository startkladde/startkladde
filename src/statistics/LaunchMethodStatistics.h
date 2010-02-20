/*
 * LaunchMethodStatistics.h
 *
 *  Created on: Aug 18, 2009
 *      Author: mherrman
 */

#ifndef LAUNCHMETHODSTATISTICS_H_
#define LAUNCHMETHODSTATISTICS_H_

// TODO reduce dependencies
#include <QString>
#include <QList>
#include <QAbstractTableModel>

class DataStorage;
class Flight;

class LaunchMethodStatistics: public QAbstractTableModel
{
	Q_OBJECT;

	public:
		class Entry
		{
			public:
				Entry ();
				virtual ~Entry ();

				QString name;
				int num;
		};

		LaunchMethodStatistics (QObject *parent=NULL);
		virtual ~LaunchMethodStatistics ();

		static LaunchMethodStatistics *createNew (const QList<Flight> &flights, DataStorage &dataStorage);

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

	private:
		QList<Entry> entries;
};

#endif /* LAUNCHMETHODSTATISTICS_H_ */
