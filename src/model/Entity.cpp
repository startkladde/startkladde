#include "Entity.h"

// ******************
// ** Construction **
// ******************

Entity::Entity ()
{
	id=0;
}

Entity::Entity (db_id id)
{
	this->id=id;
}

Entity::~Entity ()
{
}


// ********************************
// ** EntitySelectWindow helpers **
// ********************************

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
