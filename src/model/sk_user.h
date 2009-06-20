#ifndef sk_user_h
#define sk_user_h

/*
 * sk_user
 * Martin Herrmann
 * 2005-01-04
 */

using namespace std;

#include <string>
#include "src/db/db_types.h"

class sk_user
{
	public:
		sk_user ();

		string username;
		// password not saved here
		bool perm_club_admin;
		bool perm_read_flight_db;
		string club;

		db_id person;
};

#endif

