/*
 * DatabaseDoesNotExistException.cpp
 *
 *  Created on: 02.03.2010
 *      Author: Martin Herrmann
 */

#include "DatabaseDoesNotExistException.h"

namespace Db { namespace Interface
{
	DatabaseDoesNotExistException::DatabaseDoesNotExistException (const QSqlError &error):
		ConnectionFailedException (error)
	{
	}

	DatabaseDoesNotExistException *DatabaseDoesNotExistException::clone () const
	{
		return new DatabaseDoesNotExistException (error);
	}

	void DatabaseDoesNotExistException::rethrow () const
	{
		throw DatabaseDoesNotExistException (error);
	}
} }
