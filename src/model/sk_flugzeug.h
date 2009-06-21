#ifndef sk_flugzeug_h
#define sk_flugzeug_h

#include <cstdio>
#include <string>

#include "src/data_types.h"
#include "src/model/stuff.h"

using namespace std;


class sk_flugzeug: public stuff
{
	public:
		sk_flugzeug (string, string, string, string, int, db_id p_id=0);
		sk_flugzeug ();

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

