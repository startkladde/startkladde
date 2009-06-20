#include "object_field.h"

#include <stdlib.h>
#include "text.h"

void object_field::parse_text (const string &text)/*{{{*/
{
	switch (data_type)
	{
		case dt_string: current_string=text; break;
		case dt_db_id:
		{
			if (text.empty () || text=="-")
				current_db_id=invalid_id;
			else
				current_db_id=atoll (text.c_str ());
		} break;
		case dt_bool: current_bool=(atoi (text.c_str ())!=0); break;
		case dt_password: current_string=text; break;
		case dt_special: break;
	}
}
/*}}}*/

string object_field::make_text () const/*{{{*/
{
	switch (data_type)
	{
		case dt_string: return current_string; break;
		case dt_db_id: 
		{
			if (id_invalid (current_db_id))
				return "";
			else
				return num_to_string (current_db_id);
		} break;
		case dt_bool: return current_bool?"1":"0"; break;
		case dt_password: return current_string; break;
		case dt_special: return ""; break;
	}

	return "???";
}
/*}}}*/

string object_field::make_text_safe () const/*{{{*/
{
	if (data_type==dt_password)
		return "***";
	else
		return make_text ();
}
/*}}}*/

string object_field::make_display_text () const/*{{{*/
{
	switch (data_type)
	{
		case dt_string: return current_string; break;
		case dt_db_id:
		{
			if (id_invalid (current_db_id))
				return "-";
			else
				return num_to_string (current_db_id);
		} break;
		case dt_bool: return current_bool?"Ja":"Nein"; break;
		case dt_password: return "***"; break;
		case dt_special: return ""; break;
	}

	return "???";
}
/*}}}*/

void object_field::clear ()/*{{{*/
{
	switch (data_type)
	{
		case dt_string: current_string=""; break;
		case dt_db_id: current_db_id=invalid_id; break;
		case dt_bool: current_bool=false; break;
		case dt_password: current_string=""; break;
		case dt_special: break;
	}
}
/*}}}*/

