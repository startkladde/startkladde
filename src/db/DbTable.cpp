#include "DbTable.h"

dbTable::dbTable (QString _name)
	:name (_name)
{
}

dbTable::dbTable (QString _name, QString _like_table)
	:name (_name), like_table (_like_table)
{
}

std::ostream &operator<< (std::ostream &s, const dbTable &t)
{
	if (t.like_table.isEmpty ())
	{
		s << t.name << std::endl;
		QList<DbColumn>::const_iterator end=t.columns.end ();
		for (QList<DbColumn>::const_iterator it=t.columns.begin (); it!=end; ++it)
			s << "  " << *it << std::endl;
	}
	else
	{
		s << t.name << " like " << t.like_table << std::endl;
	}

	return s;
}

QString dbTable::mysql_create_query (bool force) const
{
	QString r="CREATE TABLE ";
	if (!force) r+="IF NOT EXISTS ";
	r+=name;

	if (like_table.isEmpty ())
	{
		// Create from columns and other values
		r+=" (\n";
		QList<DbColumn>::const_iterator col_begin=columns.begin ();
		QList<DbColumn>::const_iterator col_end=columns.end ();

		for (QList<DbColumn>::const_iterator cit=col_begin; cit!=col_end; ++cit)
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

			// TODO: INDEX title (title(25)) für title type text, blob
			r+=",\n";
			r+="  UNIQUE KEY "+kit+" ("+kit+")";
		}

		r+="\n";
		r+=") TYPE=MyISAM CHARSET utf8 COLLATE utf8_unicode_ci;";
	}
	else
	{
		// Create like other Table
		r+=" LIKE "+like_table;
	}

	return r;
}

