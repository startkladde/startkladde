#include "IndexSpec.h"

IndexSpec::IndexSpec (const QString &table, const QString &name, const QString &columns):
	table (table), name (name), columns (columns)
{
}

//static IndexSpec IndexSpec::singleColumn (const QString &table, const QString &column)
//{
//	return IndexSpec (table, column+"_index", column);
//}

IndexSpec::~IndexSpec ()
{
}

