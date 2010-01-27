#include "Entity.h"

Entity::Entity ()
	/*
	 * Constructs an empty Entity instance.
	 */
{
	id=0;
}

Entity::Entity (db_id id)
{
	this->id=id;
}

Entity::~Entity ()
	/*
	 * Cleans up a Entity instance.
	 */
{

}



QString Entity::get_selector_value (int column_number) const
{
	switch (column_number)
	{
		case 0: return QString::number (id);
		case 1: return comments;
		default: return QString ();
	}
}

QString Entity::get_selector_caption (int column_number)
{
	switch (column_number)
	{
		case 0: return "ID";
		case 1: return "Bemerkungen";
		default: return QString ();
	}
}


void Entity::output (std::ostream &stream, output_format_t format, bool last, QString name, QString value)
{
	switch (format)
	{
		case of_single:
		{
			stream << name << "\t" << value << std::endl;
		} break;
		case of_table_data:
		{
			QString v=value;
			replace_tabs (v);
			stream << v;
			if (!last) stream << "\t";
			if (last) stream << std::endl;
		} break;
		case of_table_header:
		{
			QString n=name;
			replace_tabs (n);
			stream << n;
			if (!last) stream << "\t";
			if (last) stream << std::endl;
		} break;
	}
}

void Entity::output (std::ostream &stream, output_format_t format, bool last, QString name, db_id value)
{
	output (stream, format, last, name, QString::number (value));
}

