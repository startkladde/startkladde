#ifndef sk_button_h
#define sk_button_h

#include <qpushbutton.h>

#include "data_types.h"
#include "db_types.h"

using namespace std;

class sk_button:public QPushButton
{
	Q_OBJECT

	public:
		sk_button (db_id, const QString &, QWidget *, const char *name=0);
	
	signals:
		void clicked (db_id, db_id);
		void clicked (db_id);
	
	private slots:
		void slot_clicked ();
        
	private:
		db_id data;
};

#endif

