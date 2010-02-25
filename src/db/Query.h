/*
 * Query.h
 *
 *  Created on: 25.02.2010
 *      Author: Martin Herrmann
 */

#ifndef QUERY_H_
#define QUERY_H_

#include <QString>
#include <QList>

class QSqlQuery;
class QVariant;

namespace Db
{
	namespace Result { class Result; }

	class Query
	{
		public:
			// *** Construction
			Query ();
			Query (const QString &queryString);
			virtual ~Query ();

			// *** Generation
			static Query selectDistinctColumns (const QString     &table , const QString     &column , bool excludeEmpty=false);
			static Query selectDistinctColumns (const QStringList &tables, const QStringList &columns, bool excludeEmpty=false);
			static Query selectDistinctColumns (const QStringList &tables, const QString     &column , bool excludeEmpty=false);
			static Query selectDistinctColumns (const QString     &table , const QStringList &columns, bool excludeEmpty=false);

			// *** Arg
			Query &arg (const QString &a1, const QString &a2);
			Query &arg (const QString &a1, const QString &a2, const QString &a3);
			Query &arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4);
			Query &arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4, const QString &a5);
			Query &arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4, const QString &a5, const QString &a6);
			Query &arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4, const QString &a5, const QString &a6, const QString &a7);
			Query &arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4, const QString &a5, const QString &a6, const QString &a7, const QString &a8);
			Query &arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4, const QString &a5, const QString &a6, const QString &a7, const QString &a8, const QString &a9);
			Query &arg (const QString &a, int fieldWidth = 0, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (QChar          a, int fieldWidth = 0, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (char           a, int fieldWidth = 0, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (int            a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (uint           a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (long           a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (ulong          a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (qlonglong      a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (qulonglong     a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (short          a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (ushort         a, int fieldWidth = 0, int base = 10, const QChar &fillChar = QLatin1Char (' '));
			Query &arg (double         a, int fieldWidth = 0, char format = 'g', int precision = -1, const QChar &fillChar = QLatin1Char (' '));

			// *** Bind
			Query &bind (const QVariant &v);

			// *** QSqlQuery interface
			void prepare (QSqlQuery &query) const;
			void bindTo (QSqlQuery &query) const;

			// *** Result
			void setResult (Result::Result *result) const;
			Result::Result *getResult () const;

		private:
			QString queryString;
			QList<QVariant> bindValues;

			mutable Result::Result *result;
	};
}

#endif
