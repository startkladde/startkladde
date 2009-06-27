#include "Condition.h"

Condition::Condition ()
{
	init ();
}

Condition::Condition (ConditionType _type)
{
	init ();
	type=_type;
}

Condition::Condition (ConditionType _type, db_id _id1)
{
	init ();
	type=_type;
	id1=_id1;
}

Condition::Condition (ConditionType _type, QString *_text1, QString *_text2)
{
	init ();
	type=_type;
	text1=_text1;
	text2=_text2;
}

Condition::Condition (ConditionType _type, const QDate *_date1, const QDate *_date2)
{
	init ();
	type=_type;
	date1=_date1;
	date2=_date2;
}

Condition::Condition (ConditionType _type, Time *_given_time1)
{
	init ();
	type=_type;
	given_time1=_given_time1;
}

Condition::Condition (ConditionType _type, db_id _id1, QDate *_date1, QDate *_date2)
{
	init ();
	type=_type;
	id1=_id1;
	date1=_date1;
	date2=_date2;
}



void Condition::init ()
{
	meta_query_table=ot_none;
	meta_query_column=NULL;
	type=cond_none;
	id1=0;
	text1=text2=NULL;
	date1=date2=NULL;
	given_time1=NULL;
}

void Condition::set_meta_query (db_object_type _table, const QString *_column)
{
	meta_query_table=_table;
	meta_query_column=_column;
}

