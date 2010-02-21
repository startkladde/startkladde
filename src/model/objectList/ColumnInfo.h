#ifndef COLUMNINFO_H_
#define COLUMNINFO_H_

#include <QString>

class ColumnInfo
{
	public:
		virtual int columnCount () const=0;
		virtual QString columnName (int columnIndex) const=0;
};

#endif
