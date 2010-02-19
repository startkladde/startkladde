#include <iostream>

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

QString version_info ()
{
	return QString (VERSION);
}

