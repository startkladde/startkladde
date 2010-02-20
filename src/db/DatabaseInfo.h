#ifndef _DatabaseInfo_h
#define _DatabaseInfo_h

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
