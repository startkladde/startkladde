#include "db_table.h"

db_table::db_table (string _name)/*{{{*/
	:name (_name)
{
}/*}}}*/

db_table::db_table (string _name, string _like_table)/*{{{*/
	:name (_name), like_table (_like_table)
{
}
/*}}}*/

ostream &operator<< (ostream &s, const db_table &t)/*{{{*/
{
	if (t.like_table.empty ())
	{
		s << t.name << endl;
		list<db_column>::const_iterator end=t.columns.end ();
		for (list<db_column>::const_iterator it=t.columns.begin (); it!=end; ++it)
			s << "  " << *it << endl;
	}
	else
	{
		s << t.name << " like " << t.like_table << endl;
	}

	return s;
}
/*}}}*/

string db_table::mysql_create_query (bool force) const/*{{{*/
{
	string r="CREATE TABLE ";
	if (!force) r+="IF NOT EXISTS ";
	r+=name;

	if (like_table.empty ())
	{
		// Create from columns and other values
		r+=" (\n";
		list<db_column>::const_iterator col_begin=columns.begin ();
		list<db_column>::const_iterator col_end=columns.end ();

		for (list<db_column>::const_iterator cit=col_begin; cit!=col_end; ++cit)
		{
			if (cit!=col_begin) r+=",\n";
			r+="  "+(*cit).mysql_spec ();
		}

		if (!primary_key.empty ())
		{
			r+=",\n";
			r+="  PRIMARY KEY ("+primary_key+")";
		}

		list<string>::const_iterator key_begin=unique_keys.begin ();
		list<string>::const_iterator key_end=unique_keys.end ();
		for (list<string>::const_iterator kit=key_begin; kit!=key_end; ++kit)
		{
			// TODO: INDEX title (title(25)) für title type text, blob
			r+=",\n";
			r+="  UNIQUE KEY "+*kit+" ("+*kit+")";
		}

		r+="\n";
		r+=") TYPE=MyISAM CHARSET=latin1;";
	}
	else
	{
		// Create like other table
		r+=" LIKE "+like_table;
	}

	return r;
}
/*}}}*/

