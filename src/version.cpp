#include <iostream>

#include <mysql.h>

#include "src/text.h"
#include "src/version.h"

void display_version ()
{
	std::cout << version_info () << std::endl;
}

void display_short_version ()
{
	std::cout << QString (VERSION) << std::endl;
}

QString version_info ()/*{{{*/
	/*
	 * Gets QString containing descriptive (for display) Version information.
	 * This information should not be used for comparison.
	 */
{
//	return QString (VERSION)+"/MySQL "+num_to_string ((int)mysql_get_client_version ());
	return QString (VERSION)+"/MySQL "+mysql_get_client_info ();
}
/*}}}*/

