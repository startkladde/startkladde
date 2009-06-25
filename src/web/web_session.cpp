#include "web_session.h"

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

web_session::web_session ()
{
	ok=false;
}

web_session::~web_session ()
{
}

web_session web_session::create ()
	/*
	 * Creates a session.
	 * That means: makes a unique file name, creates that file and saves
	 * information for uniquely identifying the session and finding the file.
	 */
{
	web_session r;

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

web_session web_session::open (QString _id)
	/*
	 * Opens a session.
	 * That means: opens the file, reads the data.
	 */
{
	web_session r;

	r.id=_id;
	bool ok=r.args.read_from_file (r.make_file_name ());
	r.ok=ok;

	return r;
}

bool web_session::save ()
	/*
	 * Saves a session.
	 * That means: writes the session data to the file.
	 */
{
	if (!ok) return false;
	return args.write_to_file (make_file_name ());
}

bool web_session::destroy ()
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



QString web_session::make_file_name ()
{
	return "/tmp/sk_web.session."+id;
}

bool web_session::is_ok ()
{
	return ok;
}

