#ifndef _SkTable_h
#define _SkTable_h

#include <QTableWidget>

#include "src/db/db_types.h"

class SkTableItem;

using namespace std;

class SkTable:public QTableWidget
{
	Q_OBJECT

	public:
		SkTable (QWidget *parent);
		void removeAllRows ();
		int row_from_column_id (db_id id, int col);
		db_id id_from_cell (int row, int col);
		SkTableItem *set_cell (int, int, const string &, QColor);
		SkTableItem *set_cell (int, int, const QString &, QColor);
		SkTableItem *set_cell (int, int, const char *, QColor);
		void setColumn (int column, QString caption, int width);

	protected:
		void keyPressEvent (QKeyEvent *);
		void simulate_key (int);
		void set_table_column (int column, string title, string sample);

	protected slots:
		virtual void columnClicked (int);

	public slots:
		virtual void removeRow (int);

	private:

	signals:
		void key (int);
};

#endif

