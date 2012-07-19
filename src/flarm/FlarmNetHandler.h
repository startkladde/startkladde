#ifndef FLARMNETHANDLER_H
#define FLARMNETHANDLER_H

#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>
#include "src/db/DbManager.h"

class FlarmNetHandler: public QObject
{
	Q_OBJECT

	public:
		// Construction/singleton
		static FlarmNetHandler* getInstance ();
		~FlarmNetHandler ();

		// Properties
		void setDatabase (DbManager*);

		void importFlarmNetDb ();
	private:
		FlarmNetHandler (QObject* parent);
		static FlarmNetHandler* instance;
		QNetworkReply* network_reply;
               
		DbManager *dbManager;

	private slots:
		void downloadFinished ();
};

#endif
