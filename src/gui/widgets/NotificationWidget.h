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

    	void setDrawWidgetBackground (bool drawWidgetBackground);
    	bool getDrawWidgetBackground () const;

    	void moveArrowTip (const QPoint &point);
    	void moveArrowTip (int x, int y);

	protected:
    	virtual void paintEvent (QPaintEvent *event);
    	virtual QSize minimumSizeHint () const;

    	QPoint getArrowTip () const;

	private:
    	Ui::NotificationWidgetClass ui;

    	int cornerRadius;

    	int arrowWidth;
    	int arrowLength;
    	int arrowPosition;

    	QColor backgroundColor;
    	QColor widgetBackgroundColor;
    	bool drawWidgetBackground;
};

#endif
