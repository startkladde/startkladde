#ifndef NOTOFICATIONWIDGET_H
#define NOTOFICATIONWIDGET_H

#include <QWidget>
#include <QRectF>
#include <QPointF>

class QHBoxLayout;
class QMouseEvent;
class QCloseEvent;
class QLabel;
class QSpacerItem;

class NotificationWidget: public QWidget
{
    Q_OBJECT

	public:
    	NotificationWidget (QWidget *parent);
    	~NotificationWidget ();

    	// Properties
    	void setFadeOutDuration (int duration);
    	int getFadeOutDuration () const;
    	void setText (const QString &text);
    	QString getText () const;
    	QWidget *contents ();

    	// Shape
    	QPointF defaultBubblePosition (const QPointF &arrowTip);

    	// Position
    	void moveTo (const QPointF &arrowTip, const QPointF &bubblePosition);
    	void moveTo (const QPointF &arrowTip);

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

    	// Layout
    	void updateLayout ();

    	// Painting
    	virtual void paintEvent (QPaintEvent *event);

    	// Interaction
    	void mousePressEvent (QMouseEvent *event);

    	// Closing
    	virtual void closeEvent (QCloseEvent *event);

	private:
    	/** Everything is in widget coordinates */
    	class Geometry
    	{
    		public:
    			Geometry (NotificationWidget *widget): widget (widget) {}

				QRectF bubble;
				QRectF northWest, northEast, southWest, southEast;

				QPointF arrowTop, arrowBottom, arrowTip;

				QPainterPath path;

				void recalculate ();

    		private:
				NotificationWidget *widget;
    	};

    	// Color properties
    	QColor bubbleColor;

    	// Shape
//    	double cornerRadius;
    	double arrowWidth;
    	QPointF arrowTipFromBubblePosition; // NB!

    	// Contents
    	QSpacerItem *_topLeftSpacer;
    	QSpacerItem *_rightSpacer;
    	QSpacerItem *_bottomSpacer;
    	QWidget *_bubbleWidget;
    	QHBoxLayout *_bubbleLayout;
    	QLabel *_contents;

    	// Closing
    	int _fadeOutDuration;
    	bool _fadeOutInProgress;

    	// Misc
    	Geometry _geometry;
};

#endif
