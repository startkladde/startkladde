#ifndef _SkListWidget_h
#define _SkListWidget_h

#include <QListWidget>

class SkListWidget:public QListWidget
{
	Q_OBJECT

	public:
		SkListWidget (QWidget *parent=NULL);
		virtual QSize sizeHint () const;
};

#endif

