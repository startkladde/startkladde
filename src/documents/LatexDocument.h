#ifndef _LatexDocument_h
#define _LatexDocument_h

/*
 * LatexDocument
 * martin
 * 2005-02-09
 */

#include <iostream>
#include <sstream>

#include <QSet>

#include "src/SkException.h"
#include "src/documents/Table.h"
#include "src/io/TempDir.h"



class LatexDocument
{
	public:
		class package
		{
			public:
				package (const QString &_name, const QString &opt0="", const QString &opt1="", const QString &opt2="", const QString &opt3="");
				QString name;
				QSet<QString> options;
				QString make_use_clause () const;
		};
		class ex_generate_error:public SkException
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
		LatexDocument ();

		void write_header ();
		void write_footer ();

		QSet<QString> document_options;
		unsigned int font_size;
		bool landscape;
		QString document_class;
		QString pagestyle;
		QString margin_top, margin_bottom, margin_right, margin_left;
		QString head_co, head_ro, head_lo, foot_co, foot_ro, foot_lo;
		QString tabcolsep;
		bool no_section_numbers;


		void add_package (const package &p);

		// TODO there should be a better structure than Table, but just create
		// another Table along with functions converting to/from object_fields
		// and the various Table types does not seem like a good idea. Maybe
		// this can be solved with the ObjectField successor.
		LatexDocument &write (const Table &tab, const TableRow &header, const QList<float> &widths);
		LatexDocument &write_text (const QString &text);
		LatexDocument &start_section (const QString &caption);
		LatexDocument &write_empty_line ();

		QString get_string ();
		QString make_pdf () throw (SkException);

	private:
		void execute_command (const QString &command, const QString &location) throw (ex_generate_error);

		// TODO remove global document
		std::ostringstream doc;
		std::ostringstream body;
		QList<package> packages;
};

#endif

