#include "QueryFailedException.h"

#include <cassert>

namespace Db { namespace Interface
{
	QueryFailedException::QueryFailedException (QSqlError error, Query query,
		QueryFailedException::Phase phase):
		error (error), query (query), phase (phase)
	{
	}

	QueryFailedException QueryFailedException::prepare (QSqlError error, Query query)
	{
		return QueryFailedException (error, query, phasePrepare);
	}

	QueryFailedException QueryFailedException::execute (QSqlError error, Query query)
	{
		return QueryFailedException (error, query, phaseExecute);
	}

	QueryFailedException *QueryFailedException::clone () const
	{
		return new QueryFailedException (error, query, phase);
	}

	void QueryFailedException::deleteAndThrow () const
	{
		QueryFailedException copy (error, query, phase);
		delete this;
		throw copy;
	}

	QString QueryFailedException::toString () const
	{
		return QString (
			"Query failed during %1:\n"
			"    Number/type   : %2/%3\n"
			"    Query         : %4\n"
			"    Database error: %5\n"
			"    Driver error  : %6")
			.arg (phaseString (phase))
			.arg (error.number ()).arg (error.type ())
			.arg (query.toString ())
			.arg (error.databaseText ())
			.arg (error.driverText ());
	}

	QString QueryFailedException::phaseString (QueryFailedException::Phase phase)
	{
		switch (phase)
		{
			case phasePrepare: return "prepare";
			case phaseExecute: return "execute";
			// no default
		}

		assert (!"Unhandled phase");
		return "?";
	}
} }
