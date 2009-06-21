#include <iostream>

#include <mysql.h>

#include "src/text.h"
#include "src/version.h"

void display_version ()
{
	cout << version_info () << endl;
}

void display_short_version ()
{
	cout << string (VERSION) << endl;
}

string version_info ()/*{{{*/
	/*
	 * Gets string containing descriptive (for display) Version information.
	 * This information should not be used for comparison.
	 */
{
//	return string (VERSION)+"/MySQL "+num_to_string ((int)mysql_get_client_version ());
	return string (VERSION)+"/MySQL "+mysql_get_client_info ();
}
/*}}}*/

