#ifndef NOTOFICATIONWIDGET_H
#define NOTOFICATIONWIDGET_H

#include <QtGui/QWidget>
#include "ui_NotificationWidget.h"

class NotificationWidget: public QWidget
{
    Q_OBJECT

	public:
    	NotificationWidget (QWidget *parent);
    	~NotificationWidget ();

	protected:
    	virtual void paintEvent (QPaintEvent *event);

	private:
    	Ui::NotificationWidgetClass ui;
};

#endif
