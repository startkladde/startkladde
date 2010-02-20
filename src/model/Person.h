#ifndef _Person_h
#define _Person_h

#include <QString>

#include "src/model/Entity.h"
#include "src/model/objectList/ObjectModel.h"

class QSqlQuery;

class Person: public Entity
{
	public:
		// *** Types
		class DefaultObjectModel: public ObjectModel<Person>
		{
			virtual int columnCount () const;
			virtual QVariant displayHeaderData (int column) const;
			virtual QVariant displayData (const Person &object, int column) const;
		};


		// *** Construction
		Person ();
		Person (db_id id);


		// *** Data
		QString vorname;
		QString nachname;
		QString club;
		QString club_id;
		QString club_id_old;


		// *** Comparison
		virtual bool operator== (const Person &o) const { return id==o.id; }
		virtual bool operator< (const Person &o) const;


		// *** Formatting
		virtual QString toString () const;
		virtual QString fullName () const;
		virtual QString formalName () const;
		virtual QString formalNameWithClub () const;


		// *** EntitySelectWindow helpers
		virtual QString get_selector_value (int column_number) const;
		static QString get_selector_caption (int column_number);


		// *** ObjectListWindow/ObjectEditorWindow helpers
		static QString objectTypeDescription () { return "Person"; }
		static QString objectTypeDescriptionDefinite () { return "die Person"; }
		static QString objectTypeDescriptionPlural () { return "Personen"; }


		// SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static Person createFromQuery (const QSqlQuery &query);
		static QString insertValueList ();
		static QString updateValueList ();
		virtual void bindValues (QSqlQuery &q) const;
		static QList<Person> createListFromQuery (QSqlQuery &query);

	private:
		void initialize ();
};

#endif
