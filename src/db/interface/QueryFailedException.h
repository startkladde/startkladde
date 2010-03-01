/*
 * QueryFailedException.h
 *
 *  Created on: 28.02.2010
 *      Author: Martin Herrmann
 */

#ifndef QUERYFAILEDEXCEPTION_H_
#define QUERYFAILEDEXCEPTION_H_

#include <QSqlError>
#include <QString>

#include "src/StorableException.h"
#include "src/db/Query.h"

namespace Db
{
	namespace Interface
	{
		class QueryFailedException: public StorableException
		{
			public:
				enum Phase { phasePrepare, phaseExecute };

				QueryFailedException (QSqlError error, Query query, Phase phase);
				static QueryFailedException prepare (QSqlError error, Query query);
				static QueryFailedException execute (QSqlError error, Query query);

				virtual QueryFailedException *clone () const;
				virtual void rethrow () const;

				QString toString () const;
				static QString phaseString (Phase phase);

				QSqlError error;
				Query query;
				Phase phase;
		};
	}
}

#endif
