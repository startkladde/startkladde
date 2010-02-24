#ifndef PLANE_H_
#define PLANE_H_

#include <cstdio>
#include <QString>
#include <QList>
#include <QSqlQuery>

#include "src/model/objectList/ObjectModel.h"

/*
 * Improvements:
 *   - addObject attribute "is towplane"
 */

#include "src/model/Entity.h"
/*
 * Aircraft: everything
 * Airplanes: motorized fixed-wing
 * Motorized aircraft: airplanes and helicopters
 * So this class should be called Aircraft
 */
class Plane: public Entity
{
	public:
		// *** Types
		enum Category { categoryNone, categoryAirplane, categoryGlider, categoryMotorglider, categoryUltralight, categoryOther };

		class DefaultObjectModel: public ObjectModel<Plane>
		{
			virtual int columnCount () const;
			virtual QVariant displayHeaderData (int column) const;
			virtual QVariant displayData (const Plane &object, int column) const;
		};


		// *** Construction
		Plane ();
		Plane (dbId id);


		// *** Data
		QString registration;
		QString club;
		int numSeats;
		QString type;
		Category category;
		QString competitionCallsign;


		// *** Property access
		virtual bool selfLaunchOnly ();
	    static QString defaultRegistrationPrefix () { return "D-"; }


		// *** Formatting
		virtual QString toString () const;
		virtual QString fullRegistration () const;
	    static bool clubAwareLessThan (const Plane &p1, const Plane &p2);


	    // *** Category methods
		static QList<Category> listCategories (bool include_invalid);
		static QString categoryText (Plane::Category category);
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

