#include "temp_dir.h"

#include "errno.h"
#include "malloc.h"
#include "stdlib.h"
#include <iostream>

temp_dir::temp_dir (const string &id)/*{{{*/
	throw (ex_create_error)
	// Name will be /tmp/${id}XXXXXX
{
	string name_template=id;
	string::const_iterator end=name_template.end ();
	for (string::iterator it=name_template.begin (); it!=end; ++it)
	{
		if (string ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-.").find (*it)==string::npos)
		{
			(*it)='_';
		}
	}

	name_template="/tmp/"+name_template+"XXXXXX";

	// Need to copy to char * manually
	int len=name_template.size ();
	char *dirname_buffer=new char[len+1];
	strcpy (dirname_buffer, name_template.c_str ());

	// Make the temporary directory
	char *dirname_ret=mkdtemp (dirname_buffer);

	if (dirname_ret)
	{
		name=dirname_ret;
		delete[] dirname_buffer;
	}
	else
	{
		delete[] dirname_buffer;
		// EINAVL: name does not end with XXXXXX
		throw ex_create_error (errno);
	}
}/*}}}*/

temp_dir::~temp_dir ()/*{{{*/
{
	if (!name.empty ())
	{
		if (name.substr (0, 5)!="/tmp/")
		{
			// For safety reasons, do not delete this.
			cerr << "Error: the directory name \""+name+"\" does not start with /tmp/.";
		}
		else if (name.find ("/../")!=string::npos)
		{
			cerr << "Error: the directory name \""+name+"\" contains /../.";
		}
		else
		{
			system ((string ("rm -r ")+name).c_str ());
			//system ((string ("touch ")+name+string (".delete")).c_str ());
		}
	}
}/*}}}*/

