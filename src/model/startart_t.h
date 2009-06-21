#ifndef startart_t_h
#define startart_t_h

/*
 * startart_t
 * Martin Herrmann
 * 2004-10-20
 */

#include <string>

#include <QStringList> // XXX

#include "src/text.h"
#include "src/db/db_types.h"
#include "src/logging/messages.h"
#include "src/model/stuff.h"

using namespace std;

// TODO: move to class (function static)
enum startart_type { sat_winch, sat_airtow, sat_self, sat_other };
string startart_type_string (startart_type t);

class startart_t:public stuff
{
	public:
		startart_t ();
		startart_t (int _id, startart_type _type, string _towplane, string _description, string _short_description, string _accelerator, string _logbook_string, bool _person_required);
		startart_t (string desc);
		void init ();
		virtual void output (ostream &stream, output_format_t format);

		virtual string bezeichnung (casus) const;
		virtual string name () const;
		virtual string text_name () const;
		virtual string tabelle_name () const;

		virtual string get_selector_value (int column_number) const;
		static string get_selector_caption (int column_number);

		bool is_airtow () const { return type==sat_airtow; }
		bool towplane_known () const { return !towplane.empty (); }

		string list_text () const;
		string get_csv_string () const { return short_description; }

		bool ok;

#define rw_property(TYPE, NAME) private: TYPE NAME; public: const TYPE get_ ## NAME () const { return NAME; }; void set_ ## NAME (TYPE value) { NAME=value; };
#define ro_property(TYPE, NAME) private: TYPE NAME; public: const TYPE get_ ## NAME () const { return NAME; };
		ro_property (db_id, id);			// 1, 4, 10, 7
		ro_property (string, towplane);				// "", "D-EIAV", "", ""
		ro_property (string, accelerator);			// A, V, G, E
		ro_property (string, description);			// Winde Akaflieg, D-EIAV, Gummiseil, Eigenstart
		ro_property (string, short_description);	// WA, AV, GS, ES
		ro_property (string, logbook_string);		// W, F, G, E
		ro_property (bool, person_required);		// true, true, true, false
		// This is for things that need to be treated differently by the program.
		ro_property (startart_type, type);			// sat_winch, sat_airtow, sat_other, sat_self
#undef rw_property
#undef ro_property
};

#endif

