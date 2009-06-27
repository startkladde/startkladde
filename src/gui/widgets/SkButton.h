#ifndef _SkButton_h
#define _SkButton_h

#include <QPushButton>

#include "src/dataTypes.h"
#include "src/db/dbTypes.h"

class SkButton:public QPushButton
{
	Q_OBJECT

	public:
		SkButton (db_id, const QString &, QWidget *, const char *name=0);

	signals:
		void clicked (db_id, db_id);
		void clicked (db_id);

	private slots:
		void slot_clicked ();

	private:
		db_id data;
};

#endif

