#ifndef sk_table_h
#define sk_table_h

// Qt3:
//#include <qtable.h>
// Qt4:
#include <Qt3Support>
#define QHeader Q3Header
#define QMemArray Q3MemArray

#include "sk_table_item.h"

using namespace std;

class sk_table:public QTable
{
	Q_OBJECT

	public:
		sk_table (QWidget *parent, const char *name=NULL);
		void clear_table ();
		int row_from_column_id (db_id id, int col);
		db_id id_from_cell (int row, int col);
		sk_table_item *set_cell (int, int, const string &, QColor);
		sk_table_item *set_cell (int, int, const QString &, QColor);
		sk_table_item *set_cell (int, int, const char *, QColor);
	
	protected:
		void keyPressEvent (QKeyEvent *);
		void simulate_key (int);
		void set_table_column (QHeader *header, int column, string title, string sample);

	protected slots:
		virtual void columnClicked (int);

	public slots:
		virtual void removeRow (int);

	private:
	
	signals:
		void key (int);
};

#endif

