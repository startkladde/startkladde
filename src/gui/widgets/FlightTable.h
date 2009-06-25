#ifndef _FlightTable_h
#define _FlightTable_h

#include <QSettings>

#include "src/data_types.h"
#include "src/db/db_proxy.h"
#include "src/db/sk_db.h"
#include "src/gui/widgets/SkButton.h"
#include "src/gui/widgets/SkTable.h"
#include "src/gui/widgets/SkTableItem.h"
#include "src/model/Flight.h"
#include "src/model/LaunchType.h"
#include "src/time/sk_time_t.h"


// Indizies zur Zuordnung von Feldern zu Tabellenspalten/*{{{*/
const int tbl_idx_registration				=0;
const int tbl_idx_flugzeug_typ				=1;
const int tbl_idx_flug_typ					=2;
const int tbl_idx_pilot						=3;
const int tbl_idx_begleiter					=4;
const int tbl_idx_startart					=5;
const int tbl_idx_startzeit					=6;
const int tbl_idx_landezeit					=7;
const int tbl_idx_flugdauer					=8;
const int tbl_idx_landungen					=9;
const int tbl_idx_startort					=10;
const int tbl_idx_zielort					=11;
const int tbl_idx_bemerkungen				=12;
const int tbl_idx_abrechnungshinweis		=13;
const int tbl_idx_editierbar				=14;
const int tbl_idx_datum						=15;
const int tbl_idx_id_display				=16;
const int tabellenspalten=17;

const int tbl_idx_id=tbl_idx_registration;
const int tbl_idx_schleppref=tbl_idx_startart;
/*}}}*/

//enum zell_typ { zt_none, zt_nothing, zt_time, zt_button, zt_missing };
enum zell_typ { zt_unhandled, zt_empty, zt_n_a, zt_button, zt_time, zt_missing, zt_invalid, zt_program_error };

class FlightTable:public SkTable
{
	Q_OBJECT

	public:
		FlightTable (sk_db *_db, QWidget *parent=0);

		db_id id_from_row (int row);
		db_id schleppref_from_row (int row);
		int row_from_id (db_id id);
		int row_from_sref (db_id sref);
		bool row_is_flight (int row);

		void update_flight (db_id, Flight *);
		void remove_flight (db_id);
		void removeRow (int);

		bool gelandete_ausblenden;
		bool weggeflogene_gekommene_anzeigen;
		bool fehlerhafte_immer;

		void update_time ();
		void update_row_time (int);
		void update_row_time (int, sk_time_t *);

		void set_anzeigedatum (QDate);

		void readSettings (QSettings&);
		void writeSettings (QSettings&);
		void setFont (const QFont&);

		void setText (int row, int column, QString text);

	protected slots:
		virtual void columnClicked (int);

	private:
		int insert_row_for_flight (Flight *f);
		void set_cell_by_type (int row, int column, zell_typ typ, QString button_text, QString zeit_text, QColor bg, db_id data, const char *);
		void set_flight (int, Flight *, db_id, bool);
		SkButton *set_button_or_text (int row, int column, bool set_button, QString text, QColor bg, db_id data);

		QDate anzeigedatum;
		sk_db *db;

	signals:
		void signal_button_start (db_id);
		void signal_button_landung (db_id);
		void signal_button_schlepplandung (db_id);
};

#endif

