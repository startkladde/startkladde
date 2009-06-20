#ifndef sk_time_edit_h
#define sk_time_edit_h

#include <qframe.h>
#include <qdatetimeedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdatetime.h>
#include <QtGui/QTimeEdit>

#include "src/logging/messages.h"
#include "src/time/sk_time.h"

using namespace std;

enum sk_te_mode { tm_time, tm_box_time, tm_time_but };

/*
 * A widget for editing a time which can aditionally display a button or a
 * check box which toggles the visibility of the time editor.
 */
// TODO remove the button because it is not well tested.
class sk_time_edit:public QFrame
{
	Q_OBJECT

	public:
		sk_time_edit (QWidget *parent, const char *name=NULL);
		void set_mode (sk_te_mode p_mode);
		void reset ();

		// QTimeEdit properties
		QTime time ();
		void set_time (sk_time);
		void null_zeit ();

		// QCheckBox properties
		void set_cbox_text (const QString &);
		void set_checked (bool);
		bool checked ();
		// TODO diese funktion �fter verwenden statt checked ();
		bool time_enabled ();

		// QPushButton properties
		void set_pbut_text (const QString &);
//		virtual void resizeEvent (QResizeEvent *);

		// TODO make private, provide accessor
		bool invert;	// Whether the check box should be inverted.
	public slots:
		void set_current_time ();

	private:
		sk_te_mode mode;

		QCheckBox *cbox;
		QTimeEdit *tedit;
		QPushButton *pbut;

	private slots:
		void update_cbox ();

	signals:
		void clicked ();	// Emitted when the button is pressed.
		void time_changed ();	// Emitted when the time is changed by the user.
		void cbox_clicked ();	// Emitted when the checkbox status is toggled.
};

#endif

