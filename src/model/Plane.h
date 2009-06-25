#ifndef _Plane_h
#define _Plane_h

#include <cstdio>
#include <QString>

#include "src/data_types.h"
#include "src/model/Entity.h"

class Plane: public Entity
{
	public:
		Plane (QString, QString, QString, QString, int, db_id p_id=0);
		Plane ();

		QString registration;
		QString wettbewerbskennzeichen;
		QString typ;
		QString club;
		int sitze;
		aircraft_category category;

		virtual QString name () const;

		void dump () const;
		virtual void output (std::ostream &stream, output_format_t format);
		virtual QString bezeichnung (casus) const;
		virtual QString text_name () const;
		virtual QString tabelle_name (bool schlepp) const;
		virtual QString tabelle_name () const;
};

#endif

