/*
 * ConnectionFailedException.cpp
 *
 *  Created on: 01.03.2010
 *      Author: Martin Herrmann
 */

#include "ConnectionFailedException.h"

#include "src/io/AnsiColors.h"

namespace Db { namespace Interface
{
	ConnectionFailedException::ConnectionFailedException (const QSqlError &error):
		SqlException (error)
	{
	}

	ConnectionFailedException *ConnectionFailedException::clone () const
	{
		return new ConnectionFailedException (error);
	}

	void ConnectionFailedException::rethrow () const
	{
		throw ConnectionFailedException (error);
	}

	QString ConnectionFailedException::toString () const
	{
		return makeString (QString ("Connection failed:"));
	}

	QString ConnectionFailedException::colorizedString () const
	{
		AnsiColors c;

		return makeColorizedString (QString (
			"%1Connection failed%2:")
			.arg (c.red ()).arg (c.reset ())
			);
	}

} }
