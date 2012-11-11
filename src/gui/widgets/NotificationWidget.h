#ifndef NOTOFICATIONWIDGET_H
#define NOTOFICATIONWIDGET_H

#include <QWidget>
#include <QRectF>
#include <QPointF>

#include "ui_NotificationWidget.h"

class QMouseEvent;
class QCloseEvent;

class NotificationWidget: public QWidget
{
    Q_OBJECT

	public:
    	NotificationWidget (QWidget *parent);
    	~NotificationWidget ();

    	// Properties
    	void setDrawWidgetBackground (bool drawWidgetBackground);
    	bool getDrawWidgetBackground () const;
    	void setFadeOutDuration (int duration);
    	int getFadeOutDuration () const;
    	void setText (const QString &text);
    	QString getText () const;

    	// Position
    	void moveArrowTip (const QPointF &point);
    	void moveArrowTip (int x, int y);

    public slots:
    	void fadeOutAndCloseIn (int delay, int duration);
    	void fadeOutAndCloseIn (int delay);
    	void fadeOutAndCloseNow ();
    	void fadeOutAndCloseNow (int duration);

    signals:
    	void closed ();

	protected:
    	// Size
    	virtual void resizeEvent (QResizeEvent *event);
    	virtual QSize sizeHint () const;
    	virtual QSize minimumSizeHint () const;

    	// Painting
    	virtual void paintEvent (QPaintEvent *event);

    	// Interaction
    	void mousePressEvent (QMouseEvent *event);

    	// Closing
    	virtual void closeEvent (QCloseEvent *event);

	private:
    	class Geometry
    	{
    		public:
    			Geometry (NotificationWidget *widget): widget (widget) {}

				QRectF bubble;
				QRectF northWest, northEast, southWest, southEast;

				QPointF arrowTop, arrowBottom, arrowTip;
				QPointF straightArrowTip;

				QPainterPath path;

				void recalculate ();

    		private:
				NotificationWidget *widget;
    	};

    	Ui::NotificationWidgetClass ui;

    	// Properties
    	int cornerRadius;
    	int arrowWidth;
    	int arrowLength;
    	int arrowOffset;

    	QColor backgroundColor;
    	QColor widgetBackgroundColor;
    	bool drawWidgetBackground;

    	Geometry geometry;

    	// Closing
    	int _fadeOutDuration;
    	bool _fadeOutInProgress;
};

#endif
