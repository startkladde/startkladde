#ifndef _SkTextBox_h
#define _SkTextBox_h

#include <cstdio>

#include <QLineEdit>

class SkTextBox:public QLineEdit
{
	Q_OBJECT

	public:
		SkTextBox (QWidget * parent=0, const char * name=0);

	protected:
		void focusOutEvent (QFocusEvent *);
		void focusInEvent (QFocusEvent *);

	signals:
		void focus_out ();
		void focus_in ();
};

#endif

