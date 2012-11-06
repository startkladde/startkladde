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

    	// Properties
    	void setDrawWidgetBackground (bool drawWidgetBackground);
    	bool getDrawWidgetBackground () const;

    	void setText (const QString &text);
    	QString getText () const;

    	QPoint getArrowTip () const;

    	void selfDestruct (int milliseconds);

    	// Geometry
    	void moveArrowTip (const QPoint &point);
    	void moveArrowTip (int x, int y);

	protected:
    	// Events
    	virtual void paintEvent (QPaintEvent *event);
    	virtual void resizeEvent (QResizeEvent *event);



    	// Size
    	virtual QSize sizeHint () const;
    	virtual QSize minimumSizeHint () const;

    protected slots:
    	void selfDestructNow ();


	private:
    	Ui::NotificationWidgetClass ui;

    	// Properties
    	int cornerRadius;
    	int arrowWidth;
    	int arrowLength;

    	QColor backgroundColor;
    	QColor widgetBackgroundColor;
    	bool drawWidgetBackground;

    	// Geometry
    	int left, right, top, bottom;
    	QPoint arrowTop, arrowBottom, arrowTip;
};

#endif
