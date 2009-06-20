#ifndef sk_combo_box_h
#define sk_combo_box_h


#include <cstdio>
#include <string>
#include <QtCore/QStringList>
#include <QtGui/QLineEdit>
#include <QtGui/QStringListModel>
#include <QtGui/QComboBox>

#include "text.h"

using namespace std;

class sk_combo_box:public QComboBox
{
	Q_OBJECT

	public:
		sk_combo_box (bool editable, QWidget * parent=0);
		void insertItem (const string &, int index=-1);
		string current_text_string ();
		void fillStringArray (string **array, int num, bool del=true);
		string edit_text_string ();
		void setCurrentItem (int);
		void insert_if_new (const string &);
		void cursor_to_end ();
		void setAutoCompletion (bool);

	protected:

//	protected slots:
//		virtual void changed (const QString &);

	protected:
		virtual void focusOutEvent (QFocusEvent *event);
		virtual void focusInEvent (QFocusEvent *event);
	
	signals:
		void focus_out ();
		void focus_in ();
};

#endif

