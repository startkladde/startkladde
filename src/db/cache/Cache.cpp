/*
 * Cache.cpp
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#include "Cache.h"

namespace Db
{
	namespace Cache
	{
		Cache::Cache (Database &db):
			db (db)
		{
		}

		Cache::~Cache ()
		{
		}
	}
}
