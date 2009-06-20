#ifndef SkListWidget_h
#define SkListWidget_h

/*
 * SkListWidget
 * martin
 * 2008-07-21
 */
#include <qlistwidget.h>

using namespace std;

class SkListWidget:public QListWidget
{
	Q_OBJECT

	public:
		SkListWidget (QWidget *parent=NULL);
		virtual QSize sizeHint () const;
};

#endif

