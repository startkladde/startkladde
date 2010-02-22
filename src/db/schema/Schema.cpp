/*
 * Schema.cpp
 *
 *  Created on: 18.02.2010
 *      Author: Martin Herrmann
 */

#include "Schema.h"

Schema::Schema (DatabaseInterface &databaseInterface):
	Migration (databaseInterface)
{
}

Schema::~Schema ()
{
}
