#ifndef _SkDialog_h
#define _SkDialog_h

#include <QDialog>

#include "src/db/DbEvent.h"

using namespace Qt;

class SkDialog:public QDialog
{
	Q_OBJECT

	public:
		SkDialog (QWidget *parent, const char *name, bool modal, WFlags f, QObject *status_dialog=NULL);

		void db_connect (QObject *);

	public slots:
		void slot_db_update (DbEvent *event);

	signals:
		void db_change (DbEvent *event);
		void db_update (DbEvent *event);
		void status (QString);
		void progress (int, int);
		void long_operation_start ();
		void long_operation_end ();
};

#endif

