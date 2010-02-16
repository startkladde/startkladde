#ifndef _Person_h
#define _Person_h

#include <QString>

#include "src/dataTypes.h"
#include "src/model/Entity.h"
#include "src/model/objectList/ObjectModel.h"

class QSqlQuery;

// TODO: move to Person and change value names
enum NamePart { nt_kein, nt_vorname, nt_nachname };

class Person: public Entity
{
	public:
		class DefaultObjectModel: public ObjectModel<Person>
		{
			virtual int columnCount () const;
			virtual QVariant displayHeaderData (int column) const;
			virtual QVariant displayData (const Person &object, int column) const;
		};

		Person ();
		Person (db_id id);
		virtual void output (std::ostream &stream, output_format_t format);

		bool operator< (const Person &o) const;
		bool operator== (const Person &o) const { return id==o.id; }

		QString vorname;
		QString nachname;
		QString club;
		QString club_id;
		QString club_id_old;
		QString landesverbands_nummer;


		virtual QString getName () const;
		virtual QString getTextName () const;
		virtual QString getTableName () const;
		virtual QString full_name () const;
		virtual QString formal_name () const;

		virtual QString get_selector_value (int column_number) const;
		static QString get_selector_caption (int column_number);

		static QString objectTypeDescription () { return "Person"; }
		static QString objectTypeDescriptionDefinite () { return "die Person"; }
		static QString objectTypeDescriptionPlural () { return "Personen"; }

		QString toString () const;

		// SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static Person createFromQuery (const QSqlQuery &query);
		static QString insertValueList ();
		static QString updateValueList ();
		void bindValues (QSqlQuery &q) const;
		static QList<Person> createListFromQuery (QSqlQuery &query);

	private:
		void initialize ();
};

#endif
