#ifndef sk_win_date_h
#define sk_win_date_h

#include <qdialog.h>
#include <qdatetime.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdatetimeedit.h>
#include <QtGui/QDateEdit>

#include "spacing.h"

using namespace std;

/*
 * A date or (date and time) editor window.
 */
class sk_win_date:public QDialog
{
	Q_OBJECT

	public:
		sk_win_date (QWidget *parent, QDate datum);
		sk_win_date (QWidget *parent, QDateTime dt);
	
	protected:
//		void resizeEvent (QResizeEvent *);

	private:
		QPushButton *but_ok, *but_cancel;
		QDateEdit *edit_datum;
		QTimeEdit *edit_zeit;
		QLabel *lbl_datum, *lbl_zeit;
		void init_common ();
	
	public slots:
		void edit_date ();
		void edit_datetime ();
		int exec ();

	private slots:
		void slot_ok ();
		void slot_cancel ();
	
	signals:
		void date_accepted (QDate);	// Emitted when the OK button is pressed.
		void datetime_accepted (QDateTime);	// Emitted when the OK button is pressed.
};

#endif

