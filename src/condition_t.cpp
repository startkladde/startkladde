#include "condition_t.h"

condition_t::condition_t ()/*{{{*/
{
	init ();
}
/*}}}*/

condition_t::condition_t (condition_type_t _type)/*{{{*/
{
	init ();
	type=_type;
}
/*}}}*/

condition_t::condition_t (condition_type_t _type, db_id _id1)/*{{{*/
{
	init ();
	type=_type;
	id1=_id1;
}
/*}}}*/

condition_t::condition_t (condition_type_t _type, string *_text1, string *_text2)/*{{{*/
{
	init ();
	type=_type;
	text1=_text1;
	text2=_text2;
}
/*}}}*/

condition_t::condition_t (condition_type_t _type, const QDate *_date1, const QDate *_date2)/*{{{*/
{
	init ();
	type=_type;
	date1=_date1;
	date2=_date2;
}
/*}}}*/

condition_t::condition_t (condition_type_t _type, sk_time_t *_given_time1)/*{{{*/
{
	init ();
	type=_type;
	given_time1=_given_time1;
}
/*}}}*/

condition_t::condition_t (condition_type_t _type, db_id _id1, QDate *_date1, QDate *_date2)/*{{{*/
{
	init ();
	type=_type;
	id1=_id1;
	date1=_date1;
	date2=_date2;
}
/*}}}*/



void condition_t::init ()/*{{{*/
{
	meta_query_table=ot_none;
	meta_query_column=NULL;
	type=cond_none;
	id1=0;
	text1=text2=NULL;
	date1=date2=NULL;
	given_time1=NULL;
}
/*}}}*/

void condition_t::set_meta_query (db_object_type _table, const string *_column)/*{{{*/
{
	meta_query_table=_table;
	meta_query_column=_column;
}
/*}}}*/

