#ifndef _SkTableItem_h
#define _SkTableItem_h

#include <cstdio>
#include <string>

#include "src/data_types.h"
#include "src/db/sk_db.h"

#include <QTableWidgetItem>

using namespace std;

class SkTableItem:public QTableWidgetItem
{
	public:
		SkTableItem ();
		SkTableItem (const QString &, QColor);
		SkTableItem (const string &, QColor);
		SkTableItem (const char *, QColor);

		void set_data (void *d) { data=d; }
		void *get_data () { return data; }

		void set_id (db_id i) { data_id=i; }
		db_id id () { return data_id; }

	private:
		void init ();
		void init (QColor bg);

		QColor background;
		db_id data_id;
		void *data;
};

#endif

