/*
 * Cache.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef CACHE_H_
#define CACHE_H_


namespace Db
{
	class Database;

	namespace Cache
	{
		class Cache
		{
			public:
				Cache (Database &db);
				virtual ~Cache ();

			private:
				Database &db;
		};
	}
}

#endif
