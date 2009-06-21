#ifndef _SkTableItem_h
#define _SkTableItem_h

#include <Qt3Support> // XXX
#define QTableItem Q3TableItem
#define QTable Q3Table

#include <cstdio>
#include <string>

#include "src/data_types.h"
#include "src/db/sk_db.h"

using namespace std;

class sk_table_item:public QTableItem
{
	public:
		sk_table_item (QTable *, const QString &, QColor);
		sk_table_item (QTable *, const string &, QColor);
		sk_table_item (QTable *, const char *, QColor);

		void paint (QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);

		void set_background (int red, int green, int blue);
		void set_background (QColor);
		QColor get_background ();

		void set_id (db_id);
		db_id id ();
		virtual int alignment () const;

		void set_data (void *d) { data=d; }
		void *get_data () { return data; }

	private:
		QColor background;
		db_id data_id;
		void *data;
};

#endif

