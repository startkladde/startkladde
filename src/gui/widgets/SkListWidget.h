#ifndef _SkListWidget_h
#define _SkListWidget_h

/*
 * SkListWidget
 * martin
 * 2008-07-21
 */

#include <QListWidget>

class SkListWidget:public QListWidget
{
	Q_OBJECT

	public:
		SkListWidget (QWidget *parent=NULL);
		virtual QSize sizeHint () const;
};

#endif

