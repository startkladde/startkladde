#ifndef temp_dir_h
#define temp_dir_h

/*
 * temp_dir
 * martin
 * 2005-02-12
 */

#include <cstring>
#include <QString>

#include "src/accessor.h"
#include "src/sk_exception.h"

class temp_dir
{
	public:
		class ex_create_error: public sk_exception/*{{{*/
		{
			public:
				ex_create_error (int _errno) { errno=_errno; desc=strerror (errno); }
				~ex_create_error () throw () {};
				QString description () const { return desc; }

				int errno;
				QString desc;
		};
/*}}}*/

		temp_dir (const QString &id="temp_dir.") throw (ex_create_error);
		~temp_dir ();

		RO_ACCESSOR (QString, name)

	private:
		QString name;
};

#endif

