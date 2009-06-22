#include "Entity.h"

Entity::Entity ()/*{{{*/
	/*
	 * Constructs an empty Entity instance.
	 */
{
	id=0;
	editierbar=true;
}
/*}}}*/

Entity::~Entity ()/*{{{*/
	/*
	 * Cleans up a Entity instance.
	 */
{

}
/*}}}*/

string entityLabel (EntityType t, casus c)/*{{{*/
	/*
	 * Returns a word describing the type of Entity.
	 * TODO: this function should be overridden in the base class instead of
	 * using EntityType t.
	 * Parameters:
	 *   - t: the Entity type.
	 *   - c: the (grammatical) case of the word.
	 * Return value:
	 *   the word.
	 */
{
	switch (t)
	{
		case st_plane:/*{{{*/
		{
			switch (c)
			{
				case cas_nominativ: return "Flugzeug"; break;
				case cas_genitiv: return "Flugzeugs"; break;
				case cas_dativ: return "Flugzeug"; break;
				case cas_akkusativ: return "Flugzeug"; break;
				default: return "[Flugzeug]"; break;
			}
		} break;/*}}}*/
		case st_person:/*{{{*/
		{
			switch (c)
			{
				case cas_nominativ: return "Person"; break;
				case cas_genitiv: return "Person"; break;
				case cas_dativ: return "Person"; break;
				case cas_akkusativ: return "Person"; break;
				default: return "[Person]"; break;
			}
		} break;/*}}}*/
		case st_startart:/*{{{*/
		{
			switch (c)
			{
				case cas_nominativ: return "Startart"; break;
				case cas_genitiv: return "Startart"; break;
				case cas_dativ: return "Startart"; break;
				case cas_akkusativ: return "Startart"; break;
				default: return "[Startart]"; break;
			}
		} break;/*}}}*/
		default:/*{{{*/
		{
			switch (c)
			{
				case cas_nominativ: return "Zeug"; break;
				case cas_genitiv: return "Zeugs"; break;
				case cas_dativ: return "Zeug"; break;
				case cas_akkusativ: return "Zeug"; break;
				default: return "[Zeug]"; break;
			}
		} break;/*}}}*/
	}
}
/*}}}*/



string Entity::get_selector_value (int column_number) const/*{{{*/
{
	switch (column_number)
	{
		case 0: return num_to_string (id);
		case 1: return bemerkungen;
		default: return string ();
	}
}
/*}}}*/

string Entity::get_selector_caption (int column_number)/*{{{*/
{
	switch (column_number)
	{
		case 0: return "ID";
		case 1: return "Bemerkungen";
		default: return string ();
	}
}
/*}}}*/


void Entity::output (ostream &stream, output_format_t format, bool last, string name, string value)/*{{{*/
{
	switch (format)
	{
		case of_single:
		{
			stream << name << "\t" << value << endl;
		} break;
		case of_table_data:
		{
			string v=value;
			replace_tabs (v);
			stream << v;
			if (!last) stream << "\t";
			if (last) stream << endl;
		} break;
		case of_table_header:
		{
			string n=name;
			replace_tabs (n);
			stream << n;
			if (!last) stream << "\t";
			if (last) stream << endl;
		} break;
	}
}
/*}}}*/

void Entity::output (ostream &stream, output_format_t format, bool last, string name, db_id value)/*{{{*/
{
	output (stream, format, last, name, num_to_string (value));
}
/*}}}*/

