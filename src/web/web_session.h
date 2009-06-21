#ifndef web_session_h
#define web_session_h

/*
 * web_session
 * martin
 * 2004-12-29
 */

#include <string>

#include "src/accessor.h"
#include "src/web/argument.h"

using namespace std;

class web_session
{
	public:
		web_session ();
		static web_session create ();
		static web_session open (string _id);
		bool is_ok ();
		RO_ACCESSOR (string, id);
		bool destroy ();
		bool save ();
		~web_session ();
		argument_list args;
		RO_ACCESSOR (string, error_description)

	private:
		string id;
		bool ok;
		string make_file_name ();
		string error_description;	// only works for creation
};

#endif

