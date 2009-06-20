#ifndef sk_text_box_h
#define sk_text_box_h

#include <Qt3Support>
#include <qlineedit.h>
#include <cstdio>

using namespace std;

class sk_text_box:public QLineEdit
{
	Q_OBJECT

	public:
		sk_text_box (QWidget * parent=0, const char * name=0);

	protected:
		void focusOutEvent (QFocusEvent *);
		void focusInEvent (QFocusEvent *);
	
//	public slots:
//		void flight_new ();
//		void flight_land ();
	
	signals:
		void focus_out ();
		void focus_in ();
};

#endif

