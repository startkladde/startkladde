#ifndef _User_h
#define _User_h

#include <QString>

#include "src/db/dbTypes.h"

class User
{
	public:
		User ();

		QString username;
		// password not saved here
		bool perm_club_admin;
		bool perm_read_flight_db;
		QString club;

		db_id person;
};

#endif

