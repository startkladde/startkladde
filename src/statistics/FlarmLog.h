#ifndef FLARMLOG_H_
#define FLARMLOG_H_

#include <QAbstractTableModel>
#include <QString>
#include <QList>

#include "src/db/dbId.h"
#include "src/flarm/FlarmHandler.h"

class FlarmLog: public QAbstractTableModel
{
		Q_OBJECT

	protected:
		FlarmLog (QObject *parent=NULL);
		~FlarmLog ();

	public:
		static FlarmLog *createNew ();

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

	private:
		QMap<QString,FlarmRecord*>* regMap;

	private slots:
		void refresh ();
};

#endif
