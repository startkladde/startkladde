#ifndef DATABASEINFO_H_
#define DATABASEINFO_H_

#include <QString>

class DatabaseInfo
{
	public:
		DatabaseInfo ();
		virtual ~DatabaseInfo ();

		virtual QString toString () const;
		virtual operator QString () const;

		QString server;
		int port;
		QString username;
		QString password;
		QString database;
};

#endif
