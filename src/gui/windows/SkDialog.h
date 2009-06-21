#ifndef _SkDialog_h
#define _SkDialog_h

#include <qnamespace.h>
#include <qdialog.h>

#include "src/db/db_event.h"

using namespace std;
using namespace Qt;

class SkDialog:public QDialog
{
	Q_OBJECT

	public:
		SkDialog (QWidget *parent, const char *name, bool modal, WFlags f, QObject *status_dialog=NULL);

		void db_connect (QObject *);

	public slots:
		void slot_db_update (db_event *event);

	signals:
		void db_change (db_event *event);
		void db_update (db_event *event);
		void status (QString);
		void progress (int, int);
		void long_operation_start ();
		void long_operation_end ();
};

#endif

