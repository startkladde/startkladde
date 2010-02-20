#ifndef _Plane_h
#define _Plane_h

#include <cstdio>
#include <QString>
#include <QList>
#include <QSqlQuery>

#include "src/model/objectList/ObjectModel.h"
#include "src/text.h" // Required for lengthSpecification

/*
 * Improvements:
 *   - addObject attribute "is towplane"
 */

#include "src/model/Entity.h"

class Plane: public Entity
{
	public:
		// *** Types
		enum Category { categoryNone, categorySingleEngine, categoryGlider, categoryMotorglider, categoryUltralight, categoryOther };

		class DefaultObjectModel: public ObjectModel<Plane>
		{
			virtual int columnCount () const;
			virtual QVariant displayHeaderData (int column) const;
			virtual QVariant displayData (const Plane &object, int column) const;
		};


		// *** Construction
		Plane ();
		Plane (db_id id);


		// *** Data
		QString registration;
		QString competitionId;
		QString type;
		QString club;
		int numSeats;
		Category category;


		// *** Property access
		virtual bool selfLaunchOnly ();
	    static QString defaultRegistrationPrefix () { return "D-"; }


		// *** Formatting
		virtual QString toString () const;
		virtual QString fullRegistration () const;
	    static bool clubAwareLessThan (const Plane &p1, const Plane &p2);


	    // *** Category methods
		static QList<Category> listCategories (bool include_invalid);
		static QString categoryText (Plane::Category category, lengthSpecification lenspec);
		static Plane::Category categoryFromRegistration (QString registration);
		static int categoryMaxSeats (Plane::Category category);


		// *** ObjectListWindow/ObjectEditorWindow helpers
		static QString objectTypeDescription () { return "Flugzeug"; }
		static QString objectTypeDescriptionDefinite () { return "das Flugzeug"; }
		static QString objectTypeDescriptionPlural () { return "Flugzeuge"; }


		// SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static Plane createFromQuery (const QSqlQuery &query);
		static QString insertValueList ();
		static QString updateValueList ();
		virtual void bindValues (QSqlQuery &q) const;
		static QList<Plane> createListFromQuery (QSqlQuery &query);
		// Enum mappers
		static QString  categoryToDb   (Category category);
		static Category categoryFromDb (QString  category);

	private:
		void initialize ();
};

#endif

