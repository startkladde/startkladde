#ifndef NOTOFICATIONWIDGET_H
#define NOTOFICATIONWIDGET_H

#include <QWidget>
#include <QRectF>
#include <QPointF>

class QHBoxLayout;
class QMouseEvent;
class QCloseEvent;
class QSpacerItem;

/**
 * Note that there is a bug which leads to the widget being moved to the wrong
 * position if moveTo is called before the widget is visible. Specifically,
 * the widget's top left corner is moved to the position where the top left
 * corner of the bubble should be. The reason seems to be that the layout is not
 * updated until the widget is shown: the bubble position is reported as (0, 0)
 * even though the margins are set up after updateLayout. The best solution
 * might be to introduce a custom layout manager. Potentially, this layout
 * manager could also make sure that the arrow and bubble positions (relative to
 * the parent widget) are maintained even if the contents are resized.
 */
class NotificationWidget: public QWidget
{
    Q_OBJECT

	public:
    	// Construction
    	NotificationWidget (QWidget *parent);
    	~NotificationWidget ();

    	// Properties
    	void setFadeOutDuration (int duration) { _fadeOutDuration=duration; }
    	int fadeOutDuration () const           { return _fadeOutDuration; }

    	// Contents
    	void setContents (QWidget *contents, bool contentsOwned);
    	QWidget *contents () const;
    	bool contentsOwned () const;
    	void setText (const QString &text);
    	QString text () const;

    	// Shape/position
    	QPointF defaultBubblePosition (const QPointF &arrowTip);
    	void moveTo (const QPointF &arrowTip, const QPointF &bubblePosition);
    	void moveTo (const QPointF &arrowTip);
    	QRectF bubbleGeometry () const;

    public slots:
    	// Closing
    	void fadeOutAndCloseIn (int delay, int duration);
    	void fadeOutAndCloseIn (int delay);
    	void fadeOutAndCloseNow ();
    	void fadeOutAndCloseNow (int duration);

    signals:
    	void closed ();

	protected:
    	virtual void resizeEvent     (QResizeEvent *event);
		virtual void paintEvent      (QPaintEvent  *event);
		virtual void mousePressEvent (QMouseEvent  *event);
		virtual void closeEvent      (QCloseEvent  *event);
    	void updateLayout ();

	private:
    	/** Everything is in widget coordinates */
    	class Shape
    	{
    		public:
    			Shape (NotificationWidget *widget);

				void invalidate ();
				void update ();

				QPainterPath path;
				QPointF arrowTip;
				QRectF bubble;

    		private:
				void recalculate ();

				NotificationWidget *_widget;
				bool _valid;
    	};

    	// Color properties
    	QColor bubbleColor;

    	// Shape parameters
    	double arrowWidth;
    	QPointF arrowTipFromBubblePosition; // NB!

    	// Calculated shape
    	const Shape &shape () const;
    	void invalidateShape ();
    	mutable Shape _shape_;

    	// Layout
//    	bool _layoutInitialized; // FIXME remove if not required
    	QSpacerItem *_topLeftSpacer;
    	QSpacerItem *_rightSpacer;
    	QSpacerItem *_bottomSpacer;
    	QHBoxLayout *_bubbleLayout;

    	// Contents
    	QWidget *_contents;
    	bool _contentsOwned;

    	// Closing
    	int _fadeOutDuration;
    	bool _fadeOutInProgress;

};

#endif
