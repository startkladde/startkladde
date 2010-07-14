/*
 * environment.cpp
 *
 *  Created on: 14.07.2010
 *      Author: Martin Herrmann
 */

#include "environment.h"

#include <stdlib.h>

/**
 * Gets the contents of an environment variable
 *
 * @param name the name of the environment variable
 * @return the contents of the environment variable
 */
QString getEnvironmentVariable (const QString &name)
{
	char *r=getenv (name.toUtf8 ().constData ());

	if (r)
		return QString (r);
	else
		return QString ();
}
