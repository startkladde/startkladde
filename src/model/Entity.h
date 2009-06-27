#ifndef _Entity_h
#define _Entity_h

#include <iostream>
#include <QString>

#include "src/dataTypes.h"
#include "src/text.h"
#include "src/db/dbTypes.h"


//#define column_string(num, title, value) if (column_number==num) { if (entry) return value; else return title; }

class Entity
/*
 * A base class for various items, like planes and persons.
 */
{
	public:
		// TODO: this concept needs work.
		// Instead of specifying different output formats here, make a
		// function that returns a list of key/value pairs
		// (ArgumentList, for example) and use that for display.
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

		Entity ();
		virtual ~Entity ();
		virtual QString bezeichnung (casus) const=0;
		virtual QString name () const=0;
		virtual QString text_name () const=0;
		virtual QString tabelle_name () const=0;
		// TODO code duplication with dump
		virtual void output (std::ostream &stream, output_format_t format)=0;

		virtual QString get_selector_value (int column_number) const;
		static QString get_selector_caption (int column_number);


		db_id id;
		bool editierbar;
		QString bemerkungen;

	protected:
		void output (std::ostream &stream, output_format_t format, bool last, QString name, QString value);
		void output (std::ostream &stream, output_format_t format, bool last, QString name, db_id value);
};

// TODO make this a static class function
// TODO this should not be necessery but be implemented via inheritance
QString entityLabel (EntityType t, casus c);

#endif

