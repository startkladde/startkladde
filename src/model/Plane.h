#ifndef _Plane_h
#define _Plane_h

#include <cstdio>
#include <QString>
#include <QList>
#include <QSqlQuery>

#include "src/model/objectList/ObjectModel.h"

/*
 * Improvements:
 *   - addObject attribute "is towplane"
 */

#include "src/dataTypes.h"
#include "src/model/Entity.h"

class Plane: public Entity
{
	public:
		class DefaultObjectModel: public ObjectModel<Plane>
		{
			virtual int columnCount () const;
			virtual QVariant displayHeaderData (int column) const;
			virtual QVariant displayData (const Plane &object, int column) const;
		};

		enum Category { categoryNone, categorySingleEngine, categoryGlider, categoryMotorglider, categoryUltralight, categoryOther };

		Plane ();
		Plane (db_id id);

		static QList<Category> listCategories (bool include_invalid);
		static QString categoryText (Plane::Category category, lengthSpecification lenspec);
		static Plane::Category categoryFromRegistration (QString registration);
		static int categoryMaxSeats (Plane::Category category);

		QString registration;
		QString competitionId;
		QString type;
		QString club;
		int numSeats;
		Category category;

		virtual QString name () const;

		virtual void output (std::ostream &stream, output_format_t format);
		virtual QString textName () const;
		virtual QString tableName () const;

		virtual bool selfLaunchOnly ();

		// TODO replace with a class Noun;
		static QString objectTypeDescription () { return "Flugzeug"; }
		static QString objectTypeDescriptionDefinite () { return "das Flugzeug"; }
		static QString objectTypeDescriptionPlural () { return "Flugzeuge"; }

	    static bool clubAwareLessThan (const Plane &p1, const Plane &p2);

	    static QString defaultRegistrationPrefix () { return "D-"; }

		QString toString () const;

		// SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static Plane createFromQuery (const QSqlQuery &query);
		static QString insertValueList ();
		static QString updateValueList ();
		void bindValues (QSqlQuery &q) const;
		static QList<Plane> createListFromQuery (QSqlQuery &query);
		// Enum mappers
		static QString  categoryToDb   (Category category);
		static Category categoryFromDb (QString  category);

	private:
		void initialize ();
};

#endif

