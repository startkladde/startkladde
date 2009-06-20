#ifndef sk_flight_table_h
#define sk_flight_table_h

// Qt3:
//#include <qmemarray.h>
// Qt4:
//#include <q3memarray.h>
#include <Qt3Support>
#include <qsettings.h>
// #include <qtimer.h>

#include "src/model/sk_flug.h"
#include "src/gui/widgets/sk_table_item.h"
#include "src/gui/widgets/sk_button.h"
#include "src/data_types.h"
#include "src/db/db_proxy.h"
#include "src/gui/widgets/sk_table.h"
#include "src/db/sk_db.h"
#include "src/model/startart_t.h"
#include "src/time/sk_time_t.h"

using namespace std;

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

class sk_flight_table:public sk_table
{
	Q_OBJECT

	public:
		sk_flight_table (sk_db *_db, QWidget *parent=0, const char *name=0);

		db_id id_from_row (int row);
		db_id schleppref_from_row (int row);
		int row_from_id (db_id id);
		int row_from_sref (db_id sref);
		bool row_is_flight (int row);

		void update_flight (db_id, sk_flug *);
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
	protected:
		QWidget *beginEdit (int row, int col, bool replace);
		int row_height;

	protected slots:
		virtual void columnClicked (int);

	private:
		int insert_row_for_flight (sk_flug *f);
		void set_cell_by_type (int row, int column, zell_typ typ, char *button_text, string zeit_text, QColor bg, db_id data, const char *);
		void set_flight (int, sk_flug *, db_id, bool);
//		int create_empty_row ();
		sk_button *set_button_or_text (int row, int column, bool set_button, QString text, QColor bg, db_id data);
		sk_button *set_button_or_text (int row, int column, bool set_button, string text, QColor bg, db_id data);
		sk_button *set_button_or_text (int row, int column, bool set_button, const char *text, QColor bg, db_id data);

		QDate anzeigedatum;
		sk_db *db;

	private slots:
//		void slot_button_landung (db_id, db_id);

	signals:
		void signal_button_start (db_id);
		void signal_button_landung (db_id);
		void signal_button_schlepplandung (db_id);
};

#endif

