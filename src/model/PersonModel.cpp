/*
 * PersonModel.cpp
 *
 *  Created on: 25.09.2010
 *      Author: martin
 */

#include "PersonModel.h"

#include "src/model/Person.h"

PersonModel::PersonModel ()
{
}

PersonModel::~PersonModel ()
{
}

int PersonModel::columnCount () const
{
	return 5;
}

QVariant PersonModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return "Nachname";
		case 1: return "Vorname";
		case 2: return "Verein";
		case 3: return "Bemerkungen";
		case 4: return "ID";
	}

	return QVariant ();
}

QVariant PersonModel::displayData (const Person &person, int column) const
{
	switch (column)
	{
		case 0: return person.lastName;
		case 1: return person.firstName;
		case 2: return person.club;
		case 3: return person.comments;
		case 4: return person.getId ();
	}

	return QVariant ();
}
