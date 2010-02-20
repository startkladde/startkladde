/*
 * Schema.cpp
 *
 *  Created on: 18.02.2010
 *      Author: Martin Herrmann
 */

#include "Schema.h"

Schema::Schema (Database &database):
	Migration (database)
{
}

Schema::~Schema ()
{
}
