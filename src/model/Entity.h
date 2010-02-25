#ifndef ENTITY_H_
#define ENTITY_H_

#include <QString>

#include "src/db/dbId.h"

namespace Db { class Database; }

/**
 * An object stored in the database
 *
 * Contains an id and comments.
 */
class Entity
{
	friend class Db::Database;

	public:
		// *** Construction
		Entity ();
		Entity (dbId id); // TODO protected (friend Database)?
		virtual ~Entity ();


		// *** Data
		QString comments;


		// *** Property access
		virtual dbId getId () const { return id; }
		virtual void setId (dbId id) { this->id=id; }


		// *** EntitySelectWindow helpers
		virtual QString get_selector_value (int column_number) const;
		static QString get_selector_caption (int column_number);

	protected:
		dbId id;
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
