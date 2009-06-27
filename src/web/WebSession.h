#ifndef _WebSession_h
#define _WebSession_h

/*
 * WebSession
 * martin
 * 2004-12-29
 */

#include <QString>

#include "src/accessor.h"
#include "src/web/Argument.h"

class WebSession
{
	public:
		WebSession ();
		static WebSession create ();
		static WebSession open (QString _id);
		bool is_ok ();
		RO_ACCESSOR (QString, id);
		bool destroy ();
		bool save ();
		~WebSession ();
		ArgumentList args;
		RO_ACCESSOR (QString, error_description)

	private:
		QString id;
		bool ok;
		QString make_file_name ();
		QString error_description;	// only works for creation
};

#endif

