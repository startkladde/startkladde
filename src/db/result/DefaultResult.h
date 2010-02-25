/*
 * DefaultResult.h
 *
 *  Created on: 25.02.2010
 *      Author: Martin Herrmann
 */

#ifndef DEFAULTRESULT_H_
#define DEFAULTRESULT_H_

#include <QSqlQuery>

#include "src/db/result/Result.h"

namespace Db
{
	namespace Result
	{
		class DefaultResult: public Result
		{
			public:
				// *** Construction
				DefaultResult (QSqlQuery &query);
				virtual ~DefaultResult ();

				// *** Result methods
				virtual int at () const;
				virtual bool first ();
				virtual bool isNull (int field) const;
				virtual bool last ();
				virtual QVariant lastInsertId () const;
				virtual QString lastQuery () const;
				virtual bool next ();
				virtual int numRowsAffected () const;
				virtual bool previous ();
				virtual QSqlRecord record () const;
				virtual bool seek (int index, bool relative=false);
				virtual int size () const;
				virtual QVariant value (int index) const;

			private:
				// Not a reference - the query will be destroyed after the
				// query method returns
				QSqlQuery query;
		};
	}
}

#endif
