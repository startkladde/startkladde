#include "Query.h"

#include <QVariant>
#include <QSqlQuery>
#include <QStringList>

namespace Db
{
	// ******************
	// ** Construction **
	// ******************

	Query::Query ()
	{
	}

	/**
	 * Note that we could write Query ("...") without this constructor, but we
	 * couldn't write interface.executeQuery ("...").
	 *
	 * @param queryString
	 * @return
	 */
	Query::Query (const char *queryString):
		queryString (queryString)
	{
	}

	Query::Query (const QString &queryString):
		queryString (queryString)
	{

	}

	Query::~Query ()
	{
	}


	// ****************
	// ** Generation **
	// ****************

	Query Query::selectDistinctColumns (const QString &table, const QString &column, bool excludeEmpty)
	{
		// "select distinct column from table"
		QString queryString=QString ("SELECT DISTINCT %1 FROM %2").arg (column, table);

		// ..." where column!=''"
		if (excludeEmpty) queryString+=QString (" WHERE %1!=''").arg (column);

		return Query (queryString);
	}

	Query Query::selectDistinctColumns (const QStringList &tables, const QStringList &columns, bool excludeEmpty)
	{
		QStringList parts;

		// TODO join queries (with potential bind values) instead of strings
		foreach (const QString &table, tables)
			foreach (const QString &column, columns)
				parts << selectDistinctColumns (table, column, excludeEmpty).queryString;

		return Query (parts.join (" UNION "));
	}

	Query Query::selectDistinctColumns (const QStringList &tables, const QString &column, bool excludeEmpty)
	{
		return selectDistinctColumns (tables, QStringList (column), excludeEmpty);
	}

	Query Query::selectDistinctColumns (const QString &table, const QStringList &columns, bool excludeEmpty)
	{
		return selectDistinctColumns (QStringList (table), columns, excludeEmpty);
	}


	// *********
	// ** Arg **
	// *********

	Query &Query::arg (const QString &a1, const QString &a2)
		{ queryString=queryString.arg (a1, a2); return *this; }
	Query &Query::arg (const QString &a1, const QString &a2, const QString &a3)
		{ queryString=queryString.arg (a1, a2, a3); return *this; }
	Query &Query::arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4)
		{ queryString=queryString.arg (a1, a2, a3, a4); return *this; }
	Query &Query::arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4, const QString &a5)
		{ queryString=queryString.arg (a1, a2, a3, a4, a5); return *this; }
	Query &Query::arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4, const QString &a5, const QString &a6)
		{ queryString=queryString.arg (a1, a2, a3, a4, a5, a6); return *this; }
	Query &Query::arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4, const QString &a5, const QString &a6, const QString &a7)
		{ queryString=queryString.arg (a1, a2, a3, a4, a5, a6, a7); return *this; }
	Query &Query::arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4, const QString &a5, const QString &a6, const QString &a7, const QString &a8)
		{ queryString=queryString.arg (a1, a2, a3, a4, a5, a6, a7, a8); return *this; }
	Query &Query::arg (const QString &a1, const QString &a2, const QString &a3, const QString &a4, const QString &a5, const QString &a6, const QString &a7, const QString &a8, const QString &a9)
		{ queryString=queryString.arg (a1, a2, a3, a4, a5, a6, a7, a8, a9); return *this; }

	Query &Query::arg (const QString &a, int fieldWidth, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, fillChar); return *this; }
	Query &Query::arg (QChar      a, int fieldWidth, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, fillChar); return *this; }
	Query &Query::arg (char       a, int fieldWidth, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, fillChar); return *this; }
	Query &Query::arg (int        a, int fieldWidth, int base, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, base, fillChar); return *this; }
	Query &Query::arg (uint       a, int fieldWidth, int base, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, base, fillChar); return *this; }
	Query &Query::arg (long       a, int fieldWidth, int base, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, base, fillChar); return *this; }
	Query &Query::arg (ulong      a, int fieldWidth, int base, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, base, fillChar); return *this; }
	Query &Query::arg (qlonglong  a, int fieldWidth, int base, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, base, fillChar); return *this; }
	Query &Query::arg (qulonglong a, int fieldWidth, int base, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, base, fillChar); return *this; }
	Query &Query::arg (short      a, int fieldWidth, int base, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, base, fillChar); return *this; }
	Query &Query::arg (ushort     a, int fieldWidth, int base, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, base, fillChar); return *this; }
	Query &Query::arg (double     a, int fieldWidth, char format, int precision, const QChar &fillChar)
		{ queryString=queryString.arg (a, fieldWidth, format, precision, fillChar); return *this; }


	// **********
	// ** Bind **
	// **********

	Query &Query::bind (const QVariant &v)
	{
		bindValues.append (v);
		return *this;
	}


	// *************************
	// ** QSqlQuery interface **
	// *************************

	void Query::prepare (QSqlQuery &query) const
	{
		query.prepare (queryString);
	}


	/**
	 * Adds the bind values to the given QSqlQuery
	 *
	 * Note that due to QSql limitations, this method may only be called from
	 * the thread that created the QSqlQuery.
	 *
	 * @param query the query the bind values will be added to
	 */
	void Query::bindTo (QSqlQuery &query) const
	{
		foreach (const QVariant &value, bindValues)
			query.addBindValue (value);
	}
}
