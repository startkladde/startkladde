#ifndef DATABASEINFO_H_
#define DATABASEINFO_H_

class DatabaseInfo
{
	public:
		DatabaseInfo () {}
		virtual ~DatabaseInfo () {}

		QString server;
		int port;
		QString username;
		QString password;
		QString database;
};

#endif
