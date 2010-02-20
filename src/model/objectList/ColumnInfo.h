#ifndef _ColumnInfo_h
#define _ColumnInfo_h

// TODO reduce dependencies
#include <QString>

class ColumnInfo
{
	public:
		virtual int columnCount () const=0;
		virtual QString columnName (int columnIndex) const=0;
};

#endif
