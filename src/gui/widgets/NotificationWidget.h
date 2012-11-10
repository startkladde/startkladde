#ifndef NOTOFICATIONWIDGET_H
#define NOTOFICATIONWIDGET_H

#include <QWidget>
#include <QRectF>
#include <QPointF>

#include "ui_NotificationWidget.h"

class QMouseEvent;

class NotificationWidget: public QWidget
{
    Q_OBJECT

	public:
    	NotificationWidget (QWidget *parent);
    	~NotificationWidget ();
    	void selfDestructIn (int milliseconds);

    	// Properties
    	void setDrawWidgetBackground (bool drawWidgetBackground);
    	bool getDrawWidgetBackground () const;
    	void setText (const QString &text);
    	QString getText () const;

    	// Position
    	void moveArrowTip (const QPointF &point);
    	void moveArrowTip (int x, int y);

	protected:
    	// Size
    	virtual void resizeEvent (QResizeEvent *event);
    	virtual QSize sizeHint () const;
    	virtual QSize minimumSizeHint () const;

    	// Painting
    	virtual void paintEvent (QPaintEvent *event);

    	// Interaction
    	void mousePressEvent (QMouseEvent *event);

    protected slots:
    	void selfDestructNow ();

	private:
    	// FIXME store pointer to parent
    	class Geometry
    	{
    		public:
				QRectF bubble;
				QRectF northWest, northEast, southWest, southEast;

				QPointF arrowTop, arrowBottom, arrowTip;
				QPointF straightArrowTip;

				QPainterPath path;

		    	// FIXME rename recalculate
				void update (const NotificationWidget *widget);
    	};

    	Ui::NotificationWidgetClass ui;

    	bool selfDestructInProgress;

    	// Properties
    	int cornerRadius;
    	int arrowWidth;
    	int arrowLength;
    	int arrowOffset;

    	QColor backgroundColor;
    	QColor widgetBackgroundColor;
    	bool drawWidgetBackground;

    	Geometry geometry;

};

#endif
