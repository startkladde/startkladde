#ifndef _Plane_h
#define _Plane_h

#include <cstdio>
#include <string>

#include "src/data_types.h"
#include "src/model/Entity.h"

using namespace std;


class Plane: public Entity
{
	public:
		Plane (string, string, string, string, int, db_id p_id=0);
		Plane ();

		string registration;
		string wettbewerbskennzeichen;
		string typ;
		string club;
		int sitze;
		aircraft_category category;

		virtual string name () const;

		void dump () const;
		virtual void output (ostream &stream, output_format_t format);
		virtual string bezeichnung (casus) const;
		virtual string text_name () const;
		virtual string tabelle_name (bool schlepp) const;
		virtual string tabelle_name () const;
};

#endif

