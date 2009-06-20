#ifndef sk_person_h
#define sk_person_h

#include <string>

#include "src/data_types.h"
#include "src/model/stuff.h"

using namespace std;

enum namens_teil { nt_kein, nt_vorname, nt_nachname };

class sk_person: public stuff
{
	public:
		sk_person ();
		sk_person (string, string);
		sk_person (string, string, string, string, string, db_id p_id=0);
		void dump () const;
		virtual void output (ostream &stream, output_format_t format);

		string vorname;
		string nachname;
		string club;
		string club_id;
		string club_id_old;
		string landesverbands_nummer;


		virtual string bezeichnung (casus) const;
		virtual string name () const;
		virtual string pdf_name () const;
		virtual string text_name () const;
		virtual string tabelle_name () const;

		virtual string get_selector_value (int column_number) const;
		static string get_selector_caption (int column_number);
};

#endif

