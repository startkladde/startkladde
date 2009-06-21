#ifndef latex_document_h
#define latex_document_h

/*
 * latex_document
 * martin
 * 2005-02-09
 */

#include <iostream>
#include <sstream>
#include <set>

#include "src/sk_exception.h"
#include "src/documents/table.h"
#include "src/io/temp_dir.h"

using namespace std;


class latex_document
{
	public:
		class package/*{{{*/
		{
			public:
				package (const string &_name, const string &opt0="", const string &opt1="", const string &opt2="", const string &opt3="");
				string name;
				set<string> options;
				string make_use_clause () const;
		};
/*}}}*/
		class ex_generate_error:public sk_exception/*{{{*/
		{
			public:
				ex_generate_error (const string &_desc):desc (_desc) {};
				~ex_generate_error () throw () {};
				string description () const { return desc; }

				string desc;
		};
/*}}}*/
		class ex_command_failed:public ex_generate_error/*{{{*/
		{
			public:
				ex_command_failed (const string &_desc, int _num, const string &_command, const string &_output, const string &_document=""):ex_generate_error (_desc), num (_num), command (_command), output (_output), document (_document) {};
				~ex_command_failed () throw () {};
				string description () const { return desc; }

				int num;
				string command;
				string output;
				string document;
		};
/*}}}*/
		latex_document ();

		void write_header ();
		void write_footer ();

		set<string> document_options;
		unsigned int font_size;
		bool landscape;
		string document_class;
		string pagestyle;
		string margin_top, margin_bottom, margin_right, margin_left;
		string head_co, head_ro, head_lo, foot_co, foot_ro, foot_lo;
		string tabcolsep;
		bool no_section_numbers;


		void add_package (const package &p);

		// TODO there should be a better structure than table, but just create
		// another table along with functions converting to/from object_fields
		// and the various table types does not seem like a good idea. Maybe
		// this can be solved with the object_field successor.
		latex_document &write (const table &tab, const table_row &header, const list<float> &widths);
		latex_document &write_text (const string &text);
		latex_document &start_section (const string &caption);
		latex_document &write_empty_line ();

		string get_string ();
		string make_pdf () throw (sk_exception);

	private:
		void execute_command (const string &command, const string &location) throw (ex_generate_error);
		ostringstream doc;
		ostringstream body;
		list<package> packages;
};

#endif

