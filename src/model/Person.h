#ifndef _Person_h
#define _Person_h

#include <QString>

#include "src/dataTypes.h"
#include "src/model/Entity.h"

// TODO: move to Person and change value names
enum NamePart { nt_kein, nt_vorname, nt_nachname };

class Person: public Entity
{
	public:
		Person ();
		Person (QString, QString);
		Person (QString, QString, QString, QString, QString, db_id p_id=0);
		void dump () const;
		virtual void output (std::ostream &stream, output_format_t format);

		QString vorname;
		QString nachname;
		QString club;
		QString club_id;
		QString club_id_old;
		QString landesverbands_nummer;


		virtual QString bezeichnung (casus) const;
		virtual QString name () const;
		virtual QString pdf_name () const;
		virtual QString text_name () const;
		virtual QString tabelle_name () const;

		virtual QString get_selector_value (int column_number) const;
		static QString get_selector_caption (int column_number);
};

#endif

