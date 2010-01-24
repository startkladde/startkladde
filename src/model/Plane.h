#ifndef _Plane_h
#define _Plane_h

#include <cstdio>
#include <QString>
#include <QList>

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

		Plane (QString, QString, QString, QString, int, db_id p_id=0);
		Plane ();

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

		void dump () const;
		virtual void output (std::ostream &stream, output_format_t format);
		virtual QString getDescription (casus) const;
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
};

#endif

