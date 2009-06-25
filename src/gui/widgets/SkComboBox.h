#ifndef _SkComboBox_h
#define _SkComboBox_h


#include <cstdio>
#include <QString>

#include <QStringList>
#include <QLineEdit>
#include <QStringListModel>
#include <QComboBox>

#include "src/text.h"

class SkComboBox:public QComboBox
{
	Q_OBJECT

	public:
		SkComboBox (bool editable, QWidget * parent=0);
		void insertItem (const QString &, int index=-1);
		QString current_text_string ();
		void fillStringArray (QString **array, int num, bool del=true);
		QString edit_text_string ();
		void setCurrentItem (int);
		void insert_if_new (const QString &);
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

