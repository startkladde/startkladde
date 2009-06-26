#include "db_table.h"

db_table::db_table (QString _name)
	:name (_name)
{
}

db_table::db_table (QString _name, QString _like_table)
	:name (_name), like_table (_like_table)
{
}

std::ostream &operator<< (std::ostream &s, const db_table &t)
{
	if (t.like_table.isEmpty ())
	{
		s << t.name << std::endl;
		QList<db_column>::const_iterator end=t.columns.end ();
		for (QList<db_column>::const_iterator it=t.columns.begin (); it!=end; ++it)
			s << "  " << *it << std::endl;
	}
	else
	{
		s << t.name << " like " << t.like_table << std::endl;
	}

	return s;
}

QString db_table::mysql_create_query (bool force) const
{
	QString r="CREATE TABLE ";
	if (!force) r+="IF NOT EXISTS ";
	r+=name;

	if (like_table.isEmpty ())
	{
		// Create from columns and other values
		r+=" (\n";
		QList<db_column>::const_iterator col_begin=columns.begin ();
		QList<db_column>::const_iterator col_end=columns.end ();

		for (QList<db_column>::const_iterator cit=col_begin; cit!=col_end; ++cit)
		{
			if (cit!=col_begin) r+=",\n";
			r+="  "+(*cit).mysql_spec ();
		}

		if (!primary_key.isEmpty ())
		{
			r+=",\n";
			r+="  PRIMARY KEY ("+primary_key+")";
		}

		QStringListIterator it (unique_keys);
		while (it.hasNext())
		{
			const QString &kit=it.next();

			// TODO: INDEX title (title(25)) f�r title type text, blob
			r+=",\n";
			r+="  UNIQUE KEY "+kit+" ("+kit+")";
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

