/*
 * LaunchTypeStatistics.h
 *
 *  Created on: Aug 18, 2009
 *      Author: mherrman
 */

#ifndef LAUNCHTYPESTATISTICS_H_
#define LAUNCHTYPESTATISTICS_H_

#include <QString>
#include <QList>
#include <QAbstractTableModel>

class DataStorage;
class Flight;

class LaunchTypeStatistics: public QAbstractTableModel
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

		LaunchTypeStatistics (QObject *parent=NULL);
		virtual ~LaunchTypeStatistics ();

		static LaunchTypeStatistics *createNew (const QList<Flight> &flights, DataStorage &dataStorage);

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

	private:
		QList<Entry> entries;
};

#endif /* LAUNCHTYPESTATISTICS_H_ */
