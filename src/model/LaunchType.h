#ifndef _LaunchType_h
#define _LaunchType_h

#include <QString>

#include <QStringList> // XXX

#include "src/text.h"
#include "src/db/dbTypes.h"
#include "src/logging/messages.h"
#include "src/model/Entity.h"

// TODO: move to class (function static)
enum startart_type { sat_winch, sat_airtow, sat_self, sat_other };
QString startart_type_string (startart_type t);

class LaunchType:public Entity
{
	public:
		LaunchType ();
		LaunchType (int _id, startart_type _type, QString _towplane, QString _description, QString _short_description, QString _accelerator, QString _logbook_string, bool _person_required);
		LaunchType (QString desc);
		void init ();
		virtual void output (std::ostream &stream, output_format_t format);

		virtual QString bezeichnung (casus) const;
		virtual QString name () const;
		virtual QString text_name () const;
		virtual QString tabelle_name () const;

		virtual QString get_selector_value (int column_number) const;
		static QString get_selector_caption (int column_number);

		bool is_airtow () const { return type==sat_airtow; }
		bool towplane_known () const { return !towplane.isEmpty (); }

		QString list_text () const;
		QString get_csv_string () const { return short_description; }

		bool ok;

#define rw_property(TYPE, NAME) private: TYPE NAME; public: TYPE get_ ## NAME () const { return NAME; }; void set_ ## NAME (TYPE value) { NAME=value; };
#define ro_property(TYPE, NAME) private: TYPE NAME; public: TYPE get_ ## NAME () const { return NAME; };
		ro_property (db_id, id);			// 1, 4, 10, 7
		ro_property (QString, towplane);				// "", "D-EIAV", "", ""
		ro_property (QString, accelerator);			// A, V, G, E
		ro_property (QString, description);			// Winde Akaflieg, D-EIAV, Gummiseil, Eigenstart
		ro_property (QString, short_description);	// WA, AV, GS, ES
		ro_property (QString, logbook_string);		// W, F, G, E
		ro_property (bool, person_required);		// true, true, true, false
		// This is for things that need to be treated differently by the program.
		ro_property (startart_type, type);			// sat_winch, sat_airtow, sat_other, sat_self
#undef rw_property
#undef ro_property
};

#endif

