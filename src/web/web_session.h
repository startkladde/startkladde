#ifndef web_session_h
#define web_session_h

/*
 * web_session
 * martin
 * 2004-12-29
 */

#include <QString>

#include "src/accessor.h"
#include "src/web/argument.h"

class web_session
{
	public:
		web_session ();
		static web_session create ();
		static web_session open (QString _id);
		bool is_ok ();
		RO_ACCESSOR (QString, id);
		bool destroy ();
		bool save ();
		~web_session ();
		argument_list args;
		RO_ACCESSOR (QString, error_description)

	private:
		QString id;
		bool ok;
		QString make_file_name ();
		QString error_description;	// only works for creation
};

#endif

