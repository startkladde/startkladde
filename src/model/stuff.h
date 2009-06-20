#ifndef stuff_h
#define stuff_h

#include "src/data_types.h"
#include <string>
#include <iostream>
#include "src/db/db_types.h"
#include "src/text.h"

using namespace std;


//#define column_string(num, title, value) if (column_number==num) { if (entry) return value; else return title; }

class stuff
/*
 * A base class for various items, like planes and persons.
 */
{
	public:
		// TODO: this concept needs work.
		// Instead of specifying different output formats here, make a
		// function that returns a list of key/value pairs
		// (argument_list, for example) and use that for display.
		// Then, this function can also be used in the web interface
		// code.
		// Note: there are problems with passwords:
		//   - when outputting to clear text, the password should not
		//     be visible (usually, sometimes it should).
		//   - when using mysql password (), the field needs special
		//     treatment.
		//   - Also, for bool fields, output must be treated different
		//     from database writing.
		enum output_format_t { of_single, of_table_header, of_table_data };

		stuff ();
		virtual ~stuff ();
		virtual string bezeichnung (casus) const=0;
		virtual string name () const=0;
		virtual string text_name () const=0;
		virtual string tabelle_name () const=0;
		// TODO code duplication with dump
		virtual void output (ostream &stream, output_format_t format)=0;

		virtual string get_selector_value (int column_number) const;
		static string get_selector_caption (int column_number);


		db_id id;
		bool editierbar;
		string bemerkungen;

	protected:
		void output (ostream &stream, output_format_t format, bool last, string name, string value);
		void output (ostream &stream, output_format_t format, bool last, string name, db_id value);
};

// TODO make this a static class function
// TODO this should not be necessery but be implemented via inheritance
string stuff_bezeichnung (stuff_type t, casus c);

#endif

