#include "db_column.h"

void db_column::init_flags ()/*{{{*/
{
	not_null=type_unsigned=zerofill=binary=auto_increment=false;
	//primary_key=unique_key=multiple_key=false;
}
/*}}}*/

db_column::db_column ()/*{{{*/
	:type (FIELD_TYPE_NULL), length (0)
{
	init_flags ();
}/*}}}*/

db_column::db_column (const string &_name, const enum_field_types _type, const unsigned int _length, const string &_def)/*{{{*/
	:name (_name), type (_type), length (_length), def (_def)
{
	init_flags ();
}
/*}}}*/

db_column::db_column (const MYSQL_FIELD &f)/*{{{*/
{
	if (f.name) name=f.name;
	type=f.type;
	length=f.length;
	if (f.def) def=string (f.def);
	not_null=(f.flags & NOT_NULL_FLAG);
	type_unsigned=(f.flags & UNSIGNED_FLAG);
	zerofill=(f.flags & ZEROFILL_FLAG);
	binary=auto_increment=(f.flags & BINARY_FLAG);
}
/*}}}*/

string db_column::type_string () const/*{{{*/
	// Now why doesn't the MySQL library have function to do this?
{
	string r="???";
	switch (type)
	{
		case FIELD_TYPE_TINY: r="TINYINT"; break;
		case FIELD_TYPE_SHORT: r="SMALLINT"; break;
		case FIELD_TYPE_LONG: r="INTEGER"; break;
		case FIELD_TYPE_INT24: r="MEDIUMINT"; break;
		case FIELD_TYPE_LONGLONG: r="BIGINT"; break;
		case FIELD_TYPE_DECIMAL: r="DECIMAL"; break;	// synonyms DEC, NUMERIC, FIXED
		case FIELD_TYPE_FLOAT: r="FLOAT"; break;
		case FIELD_TYPE_DOUBLE: r="DOUBLE"; break; // synonyms REAL, DOUBLE PRECISION
		case FIELD_TYPE_TIMESTAMP: r="TIMESTAMP"; break;
		case FIELD_TYPE_DATE: r="DATE"; break;
		case FIELD_TYPE_TIME: r="TIME"; break;
		case FIELD_TYPE_DATETIME: r="DATETIME"; break;
		case FIELD_TYPE_YEAR: r="YEAR"; break;
		case FIELD_TYPE_STRING: r="CHAR"; break;
		case FIELD_TYPE_VAR_STRING: r="VARCHAR"; break;
		case FIELD_TYPE_BLOB: r=binary?"BLOB":"TEXT"; break;
		case FIELD_TYPE_TINY_BLOB: r=binary?"TINYBLOB":"TINYTEXT"; break;
		case FIELD_TYPE_MEDIUM_BLOB: r=binary?"MEDIUMBLOB":"MEDIUMTEXT"; break;
		case FIELD_TYPE_LONG_BLOB: r=binary?"LONGBLOB":"LONGTEXT"; break;
		case FIELD_TYPE_SET: r="SET"; break;
		case FIELD_TYPE_NULL: r="NULL"; break;

		// The following are either not supported or not documented in the
		// MySQL Documention section 14.2.6
		case FIELD_TYPE_ENUM: r="ENUM"; break;
		case FIELD_TYPE_NEWDATE: r="NEWDATE"; break;
		case FIELD_TYPE_GEOMETRY: r="GEOMETRY"; break;
		// no default to allow the compiler to warn

		// More types we are not interested in
		case MYSQL_TYPE_VARCHAR: r="VARCHAR"; break;
		case MYSQL_TYPE_BIT: r="BIT"; break;
		case MYSQL_TYPE_NEWDECIMAL: r="NEWDECIMAL"; break;
	}

	if (length>0) r+="("+num_to_string (length)+")";

	return r;
}
/*}}}*/

string db_column::mysql_spec () const/*{{{*/
{
	string r;

	r=name;
	r+=" "+type_string ();
	if (!def.empty ()) r+=" "+string ("default ")+def;
	if (not_null) r+=" NOT NULL";
	if (IS_NUM (type) && type_unsigned) r+=" UNSIGNED";
	if (IS_NUM (type) && zerofill) r+=" ZEROFILL";
	if (type==FIELD_TYPE_CHAR && binary) r+=" BINARY";
	if (auto_increment) r+=" AUTO_INCREMENT";
//	if (unique_key || primary_key || multiple_key) r+=primary_key?" PRIMARY KEY":" KEY";

	return r;
}
/*}}}*/

ostream &operator<< (ostream &s, const db_column &c)/*{{{*/
{
	return s << c.mysql_spec ();
}
/*}}}*/



