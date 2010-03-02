/*
 * DatabaseDoesNotExistException.h
 *
 *  Created on: 02.03.2010
 *      Author: Martin Herrmann
 */

#ifndef DATABASEDOESNOTEXISTEXCEPTION_H_
#define DATABASEDOESNOTEXISTEXCEPTION_H_

#include "src/db/interface/exceptions/ConnectionFailedException.h"

namespace Db
{
	namespace Interface
	{
		class DatabaseDoesNotExistException: public ConnectionFailedException
		{
			public:
				DatabaseDoesNotExistException (const QSqlError &error);

				virtual DatabaseDoesNotExistException *clone () const;
				virtual void rethrow () const;
		};
	}
}

#endif /* DATABASEDOESNOTEXISTEXCEPTION_H_ */
