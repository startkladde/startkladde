#ifndef _LaunchType_h
#define _LaunchType_h

#include <QString>
#include <QStringList>

#include "src/text.h"
#include "src/db/dbTypes.h"
#include "src/logging/messages.h"
#include "src/model/Entity.h"

class QSqlQuery;

// TODO: move to class (function static)
enum startart_type { sat_winch, sat_airtow, sat_self, sat_other };
QString startart_type_string (startart_type t);

class LaunchType:public Entity
{
	public:
		LaunchType ();
		// TODO cleanup: only () and (id) (after reading from db)
		LaunchType (db_id id);
		// FIXME remove
//		LaunchType (int _id, startart_type _type, QString _towplane, QString _description, QString _short_description, QString _accelerator, QString _logbook_string, bool _person_required);
		LaunchType (QString desc);
		void init ();
		virtual void output (std::ostream &stream, output_format_t format);
		QString toString () const;

		virtual QString name () const;
		virtual QString textName () const;
		virtual QString tableName () const;

		virtual QString get_selector_value (int column_number) const;
		static QString get_selector_caption (int column_number);

		bool is_airtow () const { return type==sat_airtow; }
		bool towplane_known () const { return !towplane.isEmpty (); }

		QString list_text () const;

		static bool nameLessThan (const LaunchType &l1, const LaunchType &l2) { return l1.description<l2.description; }

		bool ok;

		//In Entity: db_id id;			// 1, 4, 10, 7
		QString towplane;				// "", "D-EIAV", "", ""
		QString accelerator;			// A, V, G, E
		QString description;			// Winde Akaflieg, D-EIAV, Gummiseil, Eigenstart
		QString short_description;	// WA, AV, GS, ES
		QString logbook_string;		// W, F, G, E
		bool person_required;		// true, true, true, false
		startart_type type;			// sat_winch, sat_airtow, sat_other, sat_self
		QString comments;

		static QString objectTypeDescription () { return "Startart"; }
		static QString objectTypeDescriptionDefinite () { return "die Startart"; }
		static QString objectTypeDescriptionPlural () { return "Startarten"; }

		// SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static LaunchType createFromQuery (const QSqlQuery &query);
		static QString insertValueList ();
		static QString updateValueList ();
		void bindValues (QSqlQuery &q) const;
		static QList<LaunchType> createListFromQuery (QSqlQuery &query);
		// Enum mappers
		static QString       typeToDb   (startart_type category);
		static startart_type typeFromDb (QString       category);
};

#endif

