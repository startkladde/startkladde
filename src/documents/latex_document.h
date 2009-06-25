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



class latex_document
{
	public:
		class package
		{
			public:
				package (const QString &_name, const QString &opt0="", const QString &opt1="", const QString &opt2="", const QString &opt3="");
				QString name;
				std::set<QString> options;
				QString make_use_clause () const;
		};
		class ex_generate_error:public sk_exception
		{
			public:
				ex_generate_error (const QString &_desc):desc (_desc) {};
				~ex_generate_error () throw () {};
				QString description () const { return desc; }

				QString desc;
		};
		class ex_command_failed:public ex_generate_error
		{
			public:
				ex_command_failed (const QString &_desc, int _num, const QString &_command, const QString &_output, const QString &_document=""):ex_generate_error (_desc), num (_num), command (_command), output (_output), document (_document) {};
				~ex_command_failed () throw () {};
				QString description () const { return desc; }

				int num;
				QString command;
				QString output;
				QString document;
		};
		latex_document ();

		void write_header ();
		void write_footer ();

		std::set<QString> document_options;
		unsigned int font_size;
		bool landscape;
		QString document_class;
		QString pagestyle;
		QString margin_top, margin_bottom, margin_right, margin_left;
		QString head_co, head_ro, head_lo, foot_co, foot_ro, foot_lo;
		QString tabcolsep;
		bool no_section_numbers;


		void add_package (const package &p);

		// TODO there should be a better structure than table, but just create
		// another table along with functions converting to/from object_fields
		// and the various table types does not seem like a good idea. Maybe
		// this can be solved with the object_field successor.
		latex_document &write (const table &tab, const table_row &header, const std::list<float> &widths);
		latex_document &write_text (const QString &text);
		latex_document &start_section (const QString &caption);
		latex_document &write_empty_line ();

		QString get_string ();
		QString make_pdf () throw (sk_exception);

	private:
		void execute_command (const QString &command, const QString &location) throw (ex_generate_error);

		// TODO remove global document
		std::ostringstream doc;
		std::ostringstream body;
		std::list<package> packages;
};

#endif

