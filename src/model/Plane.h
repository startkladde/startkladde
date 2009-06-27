#ifndef _Plane_h
#define _Plane_h

#include <cstdio>
#include <QString>
#include <QList>

#include "src/dataTypes.h"
#include "src/model/Entity.h"

class Plane: public Entity
{
	public:
		enum Category { categoryNone, categorySingleEngine, categoryGlider, categoryMotorglider, categoryUltralight, categoryOther };

		Plane (QString, QString, QString, QString, int, db_id p_id=0);
		Plane ();

		static QList<Category> listCategories (bool include_invalid);
		static QString categoryString (Plane::Category category, lengthSpecification lenspec);
		static Plane::Category categoryFromRegistration (QString registration);

		QString registration;
		QString wettbewerbskennzeichen;
		QString typ;
		QString club;
		int sitze;
		Category category;

		virtual QString name () const;

		void dump () const;
		virtual void output (std::ostream &stream, output_format_t format);
		virtual QString bezeichnung (casus) const;
		virtual QString text_name () const;
		virtual QString tabelle_name (bool schlepp) const;
		virtual QString tabelle_name () const;
};

#endif

