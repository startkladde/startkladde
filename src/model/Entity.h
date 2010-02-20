#ifndef _Entity_h
#define _Entity_h

#include <iostream>
#include <QString>

// TODO reduce dependencies
#include "src/text.h"
#include "src/db/dbId.h"


/**
 * An object stored in the database
 *
 * Contains an id and comments.
 */
class Entity
{
	friend class Database;

	public:
		// *** Construction
		Entity ();
		Entity (db_id id); // TODO protected (friend Database)?
		virtual ~Entity ();


		// *** Data
		QString comments;


		// *** Property access
		virtual db_id getId () const { return id; }
		virtual void setId (db_id id) { this->id=id; }


		// *** EntitySelectWindow helpers
		virtual QString get_selector_value (int column_number) const;
		static QString get_selector_caption (int column_number);

	protected:
		db_id id;
};

#endif


//#include <QHash>
//template<EnumType, DatabaseType> class EnumMap
//{
//	public:
//		EnumMap (EnumValue unknownEnum, DatabaseValue unknownDatabase)
//		{
//			self->unknownEnum     = unknownEnum    ;
//			self->unknownDatabase = unknownDatabase;
//		}
//
//		void addValue (EnumType enumValue, DatabaseType databaseValue)
//		{
//			databaseValues[    enumValue] = databaseValue;
//			    enumValues[databaseValue] =     enumValue;
//		}
//
//		void toEnum (DatabaseType databaseValue) const
//		{
//			if (enumValues.contains (databaseValue))
//				return enumValues[databaseValue];
//			else
//				return unknownEnum;
//		}
//
//		void toDatabase (EnumType enumValue) const
//		{
//			if (databaseValues.contains (enumValue))
//				return databaseValues[enumValue];
//			else
//				return unknownDatabase;
//		}
//
//	private:
//		QHash<    EnumType, DatabaseType> databaseValues;
//		QHash<DatabaseType,     EnumType>     enumValues;
//
//		EnumType unknownEnum;
//		DatabaseType unknownDatabase;
//};
