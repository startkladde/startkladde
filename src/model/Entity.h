#ifndef _Entity_h
#define _Entity_h

#include <iostream>
#include <QString>

#include "src/dataTypes.h"
#include "src/text.h"
#include "src/db/dbTypes.h"


//#define column_string(num, title, value) if (column_number==num) { if (entry) return value; else return title; }

class Entity
/*
 * A base class for various items, like planes and persons.
 */
{
	public:
		// TODO: this should be replaced by ObjectItemModels
		// Note: there are problems with passwords:
		//   - when outputting to clear text, the password should not
		//     be visible (usually, sometimes it should).
		//   - when using mysql password (), the field needs special
		//     treatment.
		//   - Also, for bool fields, output must be treated different
		//     from database writing.
		enum output_format_t { of_single, of_table_header, of_table_data };

		Entity ();
		Entity (db_id id);
		virtual ~Entity ();
		virtual QString getName () const=0;
		virtual QString getTextName () const=0;
		virtual QString getTableName () const=0;
		// TODO code duplication with dump
		virtual void output (std::ostream &stream, output_format_t format)=0;

		virtual QString get_selector_value (int column_number) const;
		static QString get_selector_caption (int column_number);

		db_id get_id () const { return id; }

		db_id id;
		QString comments;

	protected:
		void output (std::ostream &stream, output_format_t format, bool last, QString name, QString value);
		void output (std::ostream &stream, output_format_t format, bool last, QString name, db_id value);
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
