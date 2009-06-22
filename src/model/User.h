#ifndef _User_h
#define _User_h

/*
 * User
 * Martin Herrmann
 * 2005-01-04
 */

using namespace std;

#include <string>

#include "src/db/db_types.h"

class User
{
	public:
		User ();

		string username;
		// password not saved here
		bool perm_club_admin;
		bool perm_read_flight_db;
		string club;

		db_id person;
};

#endif

