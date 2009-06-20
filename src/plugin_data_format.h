#ifndef plugin_data_format_h
#define plugin_data_format_h

/*
 * plugin_data_format
 * martin
 * 2005-03-20
 */

#include <string>
#include <list>
#include "accessor.h"
#include "argument.h"
#include "object_field.h"
#include "startart_t.h"
#include "sk_flug.h"
#include "sk_person.h"
#include "sk_flugzeug.h"

using namespace std;

// Not using qlibrary because it does strange things to the name (resulting in
// "liblibfoo.so.so" when copying) and has poor error reporting.

// Functions starting with "plugin" call the corresponding library function.

class plugin_data_format
{
	public:
		// Exceptions
		class exception: public std::exception/*{{{*/
		{
			public:
				exception (const string &_desc) :desc (_desc) {}
				~exception () throw () {}
				virtual string description () { return desc; }
				string desc;
		};/*}}}*/
		class ex_file_not_found: public exception/*{{{*/
		{
			public:
				ex_file_not_found (const string &name=""):exception (name.empty ()?"Plugin-Datei nicht gefunden":"Plugin-Datei "+name+" nicht gefunden") {};
		};/*}}}*/
		class ex_load_error: public exception/*{{{*/
		{
			public:
				ex_load_error (const string &msg=""):exception (msg.empty ()?"Fehler beim Laden der Bibliothek":"Fehler beim Laden der Bibliothek: "+msg) {};
		};/*}}}*/
		class ex_symbol_not_found: public exception/*{{{*/
		{
			public:
				ex_symbol_not_found (const string &name):exception ("Symbol "+name+" nicht gefunden") {};
		};/*}}}*/
		class ex_plugin_internal_error: public exception/*{{{*/
		{
			public:
				ex_plugin_internal_error (const string &msg):exception (msg), fatal (true) {};
				ex_plugin_internal_error &is_fatal (bool f=true) { fatal=f; return *this; }
				bool fatal;
				
		};/*}}}*/
		class ex_plugin_invalid_format: public ex_plugin_internal_error/*{{{*/
		{
			public:
				ex_plugin_invalid_format (const string &fmt=""): ex_plugin_internal_error (fmt.empty ()?"Ungültiges Format":"Ungültiges Format \""+fmt+"\"") { fatal=true; }
		};/*}}}*/

		// Construction
		plugin_data_format (const string &_filename);
		plugin_data_format (const plugin_data_format &p);
		~plugin_data_format ();

		// Library handling
		void load () const;
		RO_ACCESSOR (string, filename)
		RO_ACCESSOR (string, real_filename)
		RO_ACCESSOR (bool, loaded)
		string get_display_filename () const { if (real_filename.empty ()) return filename; else return real_filename; }

		// Metainformation
		argument_list plugin_list_formats () const;
		bool provides_format (const string &label) const;
		// Unique names have the filename or something appended so they can be
		// distiguished from other plugins using the same format name.
		argument_list plugin_list_unique_formats () const;
		bool provides_unique_format (const string &label) const;
		string make_unique (const string &label) const;
		string make_non_unique (const string &label) const;

		// Actual plugin functions
		void plugin_make_field_list (const string &format, list<object_field> &fields) const;
		void plugin_flight_to_fields (const string &format, list<object_field> &fields, const sk_flug &f, const sk_flug_data &flight_data, int &num, const string &none_text="", const string &error_text="") const;

	private:
		// Library handling
		string filename;
		mutable bool loaded;
		mutable void *handle;
		mutable string real_filename;
	
#define PLUGIN_SYMBOL(NAME, RETVAL, ARGLIST)	\
		typedef RETVAL (*type_ ## NAME) ARGLIST;	\
		mutable type_ ## NAME symbol_ ## NAME;
	
		PLUGIN_SYMBOL (list_formats, argument_list, ())
		PLUGIN_SYMBOL (make_field_list, void, (const string &format, list<object_field> &fields));
		PLUGIN_SYMBOL (flight_to_fields, void, (const string &format, list<object_field> &fields, const sk_flug &f, const sk_flug_data &flight_data, int &num, const string &none_text, const string &error_text));
#undef PLUGIN_SYMBOL

};

#endif

