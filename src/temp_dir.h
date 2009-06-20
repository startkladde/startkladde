#ifndef temp_dir_h
#define temp_dir_h

/*
 * temp_dir
 * martin
 * 2005-02-12
 */

#include <string>
#include <string.h>
#include "accessor.h"
#include "sk_exception.h"

using namespace std;

class temp_dir
{
	public:
		class ex_create_error: public sk_exception/*{{{*/
		{
			public:
				ex_create_error (int _errno) { errno=_errno; desc=strerror (errno); }
				~ex_create_error () throw () {};
				string description () const { return desc; }

				int errno;
				string desc;
		};
/*}}}*/

		temp_dir (const string &id="temp_dir.") throw (ex_create_error);
		~temp_dir ();

		RO_ACCESSOR (string, name)

	private:
		string name;
};

#endif

