#include "WebSession.h"

#include <cstring>
#include <fstream>
#include <iostream>

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "src/text.h"

// TODO expire sessions

WebSession::WebSession ()
{
	ok=false;
}

WebSession::~WebSession ()
{
}

WebSession WebSession::create ()
	/*
	 * Creates a session.
	 * That means: makes a unique file name, creates that file and saves
	 * information for uniquely identifying the session and finding the file.
	 */
{
	WebSession r;

	// TODO use libc macro for /tmp
	// TODO don't use /tmp here because files may be delete there
	char filename_buffer[]="/tmp/sk_web.session.XXXXXX";
	umask (0066);
	int file_handle=mkstemp (filename_buffer);
	close (file_handle);

	if (file_handle==-1)
	{
		r.error_description=strerror (errno);
		perror ("Creating a session file");
		r.ok=false;
	}
	else
	{
		r.id=QString (filename_buffer+strlen (filename_buffer)-6);
		r.ok=true;
	}

	return r;
}

WebSession WebSession::open (QString _id)
	/*
	 * Opens a session.
	 * That means: opens the file, reads the data.
	 */
{
	WebSession r;

	r.id=_id;
	bool ok=r.args.read_from_file (r.make_file_name ());
	r.ok=ok;

	return r;
}

bool WebSession::save ()
	/*
	 * Saves a session.
	 * That means: writes the session data to the file.
	 */
{
	if (!ok) return false;
	return args.write_to_file (make_file_name ());
}

bool WebSession::destroy ()
	/*
	 * Destroys a session.
	 * That means: deletes the file.
	 */
{
	int ret=unlink (make_file_name ().latin1());
	if (ret!=0) perror ("Deleting the session file");
	ok=false;
	return ret==0;
}



QString WebSession::make_file_name ()
{
	return "/tmp/sk_web.session."+id;
}

bool WebSession::is_ok ()
{
	return ok;
}

