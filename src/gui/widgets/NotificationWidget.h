#ifndef NOTOFICATIONWIDGET_H
#define NOTOFICATIONWIDGET_H

#include <QWidget>
#include <QRectF>
#include <QPointF>

#include "ui_NotificationWidget.h"

class NotificationWidget: public QWidget
{
    Q_OBJECT

	public:
    	NotificationWidget (QWidget *parent);
    	~NotificationWidget ();
    	void selfDestruct (int milliseconds);

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

    	// Geometry
    	virtual void updateGeometry ();

    	// Painting
    	virtual void paintEvent (QPaintEvent *event);

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

    	struct
    	{
			QRectF bubble;
			QRectF northWest, northEast, southWest, southEast;

			QPointF arrowTop, arrowBottom, arrowTip;
    	} geometry;

};

#endif
