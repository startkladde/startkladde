#ifndef ENTITY_H_
#define ENTITY_H_

#include <QString>

#include "src/db/dbId.h"

class Database;
class Query;
class Result;

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
		Entity (dbId id); // TODO protected (friend Database)?
		virtual ~Entity ();


		// *** Data
		QString comments;


		// *** Property access
		virtual dbId getId () const { return id; }
		virtual void setId (dbId id) { this->id=id; }	
        virtual QString getDisplayName () const=0;

        /**
        *\brief Add key-value pair to comment field
        *
        * Adds a key value pair of the form
        *    <key>= '<value>'
        * to the comments of this object. If the pair already exists, the value
        * is replaced by the new value. If value is empty, the entire key value
        * pair is removed.
        *
        *\param[in] name Key name
        *\param[in] value Value
        *
        *\author Claas H. Koehler
        *\date December 2014
        */
        virtual void setField(const char* name, const QString& value);

        /**
        *\brief Get key-value pair from comment field
        *
        * Extracts a key value pair of the form
        *    <key>= '<value>'
        * from the comments of this object.
        *
        *\param[in] name Key name
        *\return Value. If key is not found, an empty string is returned.
        *
        *\author Claas H. Koehler
        *\date December 2014
        */
        virtual QString getField(const char* name) const;

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
