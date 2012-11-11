#include "NotificationWidget.h"

#include <iostream>

#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QMouseEvent>

#include "src/gui/WidgetFader.h"
#include "src/util/qRectF.h"


// Note that this class uses QPointF and QRectF rather than QPoint and QRect
// due to obscure bottom/right/center calculation rules of QRect.
//
// Possible improvements:
//   * better behavior on moveArrowTip if the widget does not fit inside the parent

// ******************
// ** Construction **
// ******************

NotificationWidget::NotificationWidget (QWidget *parent): QWidget (parent),
	cornerRadius (10),
	arrowWidth (10), arrowLength (20), arrowOffset (0),
	backgroundColor (QColor (0, 0, 0, 191)),
	widgetBackgroundColor (QColor (0, 0, 0, 63)),
	drawWidgetBackground (false),
	_fadeOutDuration (1000), _fadeOutInProgress (false)
{
	ui.setupUi (this);

	QLayout *l=layout ();
	if (l)
	{
		// Set the corner radius. We'll use the margin of the layout, which is
		// platform/style dependent. The layout has 4 margins (one on each
		// side), but we only have one corner radius. We'll arbitrarily pick the
		// left one.
		cornerRadius=l->contentsMargins ().left ();

		// Set the layout's margins. We use the corner radius, except on the
		// left side, where we'll add space for the arrow.
		l->setContentsMargins (arrowLength+cornerRadius,
			cornerRadius, cornerRadius, cornerRadius);
	}

	// Make label text white
	QPalette widgetPalette=palette ();
	widgetPalette.setColor (QPalette::WindowText, Qt::white);
	widgetPalette.setColor (QPalette::Text      , Qt::white);
	setPalette (widgetPalette);

	// Make the widget as compact as possible
	adjustSize ();

	// The geometry will be updated in the resize event before the widget is
	// shown, but we need it now so the widget can be moved before it is shown.
	geometry.update (this);
}

NotificationWidget::~NotificationWidget()
{
}


// *************
// ** Closing **
// *************

void NotificationWidget::closeEvent (QCloseEvent *event)
{
	QWidget::closeEvent (event);
	if (event->isAccepted ())
		emit closed ();
}

void NotificationWidget::fadeOutAndCloseIn (int delay)
{
	QTimer::singleShot (delay, this, SLOT (fadeOutAndCloseNow ()));
}

void NotificationWidget::fadeOutAndCloseIn (int delay, int duration)
{
	_fadeOutDuration=duration;
	fadeOutAndCloseIn (delay);

}

void NotificationWidget::fadeOutAndCloseNow ()
{
	if (_fadeOutInProgress)
		return;
	_fadeOutInProgress=true;

	WidgetFader::fadeOutAndClose (this, _fadeOutDuration);
}

void NotificationWidget::fadeOutAndCloseNow (int duration)
{
	_fadeOutDuration=duration;
	fadeOutAndCloseNow ();
}


// ****************
// ** Properties **
// ****************

void NotificationWidget::setDrawWidgetBackground (bool drawWidgetBackground)
{
	this->drawWidgetBackground=drawWidgetBackground;
}

bool NotificationWidget::getDrawWidgetBackground () const
{
	return drawWidgetBackground;
}

void NotificationWidget::setFadeOutDuration (int duration)
{
	_fadeOutDuration=duration;
}

int NotificationWidget::getFadeOutDuration () const
{
	return _fadeOutDuration;
}

void NotificationWidget::setText (const QString &text)
{
	ui.textLabel->setText (text);
	adjustSize ();
}

QString NotificationWidget::getText () const
{
	return ui.textLabel->text ();
}


// **************
// ** Geometry **
// **************

void NotificationWidget::Geometry::update (const NotificationWidget *widget)
{
	// Calculate the extents of the bubble
	bubble=QRectF (widget->arrowLength, 0, widget->width ()-widget->arrowLength, widget->height ());

	// Calculate the corner rectangles
	northWest=northWestCorner (bubble, 2*widget->cornerRadius);
	northEast=northEastCorner (bubble, 2*widget->cornerRadius);
	southWest=southWestCorner (bubble, 2*widget->cornerRadius);
	southEast=southEastCorner (bubble, 2*widget->cornerRadius);

	// Calculate the arrow coordinates
	// This currently places the arrow immediately below the top left corner.
	arrowTop        =QPointF (bubble.left (),                     widget->cornerRadius);
	arrowBottom     =QPointF (bubble.left (),                     widget->cornerRadius+widget->arrowWidth);
	straightArrowTip=QPointF (bubble.left ()-widget->arrowLength, widget->cornerRadius+widget->arrowWidth/2);
	arrowTip        =straightArrowTip+QPointF (0, widget->arrowOffset);

	// Draw the bubble outline counter-clockwise, starting with the bottom left
	// corner arc and ending after the top left corner arc.
	path=QPainterPath ();
	path.moveTo (southWest.topLeft     ()); path.arcTo (southWest, 180, 90);
	path.lineTo (southEast.bottomLeft  ()); path.arcTo (southEast, 270, 90);
	path.lineTo (northEast.bottomRight ()); path.arcTo (northEast,   0, 90);
	path.lineTo (northWest.topRight    ()); path.arcTo (northWest,  90, 90);

	// Draw the arrow
	path.lineTo (arrowTop);
	path.lineTo (arrowTip);
	path.lineTo (arrowBottom);

	path.closeSubpath ();

//	qDebug () << "Arrow tip is at" << geometry.arrowTip;
}

// **************
// ** Position **
// **************

void NotificationWidget::moveArrowTip (const QPointF &point)
{
	// Calculate the position where this widget would have to be in order to
	// move the tip of a straight arrow to the given point.
	QPointF position=point-geometry.straightArrowTip;

	// If this widget has a parent widget (i. e. it is not a top level widget),
	// make sure it is completely inside the parent widget. Note that if the
	// parent widget is not high enough for this widget plus the margins, the
	// results may be unexpected.
	// FIXME disabled for now, it doesn't work if the arrow tip ends out outside
	// of the widget boundaries (i. e. above or below the bubble) - we'll have
	// to resize the widget in this case.
//	QWidget *parent=parentWidget ();
//	if (parent)
//	{
//		// Set the desired margin at the top and at the bottom
//		int topMargin=1, bottomMargin=1;
//
//		// Calculate the minimum and maximum y position
//		int minY=topMargin;
//		int maxY=parent->height ()-bottomMargin-height ();
//
//		// Calculate the offset to move the widget to get it inside the
//		// allowable range.
//		int yOffset=0;
//		if (position.y ()<minY)
//			yOffset=minY-position.y ();
//		else if (position.y ()>maxY)
//			yOffset=maxY-position.y ();
//
//		// Move the position
//		position += QPointF (0, yOffset);
//
//		// Update the arrow offset. If the arrow offset changed, we have to
//		// recalculate the geometry.
//		if (arrowOffset!=-yOffset)
//		{
//			arrowOffset=-yOffset;
//			geometry.update (this);
//		}
//	}

	// Move the widget
	move (position.toPoint ());
}

void NotificationWidget::moveArrowTip (int x, int y)
{
	moveArrowTip (QPointF (x, y));
}



// **********
// ** Size **
// **********

void NotificationWidget::resizeEvent (QResizeEvent *event)
{
	(void)event;
	geometry.update (this);
}

QSize NotificationWidget::minimumSizeHint () const
{
	QSize hint=QWidget::minimumSizeHint ();

	// Make sure the widget is wide enough for two rounded corners, and high
	// enough for two rounded corners and the arrow.
	hint.setWidth  (qMax (hint.width  (), 2*cornerRadius));
	hint.setHeight (qMax (hint.height (), 2*cornerRadius+arrowWidth));

	return hint;
}

QSize NotificationWidget::sizeHint () const
{
	return minimumSizeHint ();
}


// **************
// ** Painting **
// **************

void NotificationWidget::paintEvent (QPaintEvent *event)
{
	(void)event;

	QPainter painter (this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen (Qt::NoPen);

	// If we are supposed to draw the widget background, draw a rectangle
	// comprising the whole widget in the widget background color.
	if (drawWidgetBackground)
	{
		painter.setBrush (widgetBackgroundColor);
		painter.drawRect (this->rect ());
	}

	painter.setBrush (QColor (0, 0, 0, 191));
	painter.drawPath (geometry.path);
}


// *****************
// ** Interaction **
// *****************

void NotificationWidget::mousePressEvent (QMouseEvent *event)
{
	if (geometry.path.contains (event->posF ()))
		// Act on the event
		close ();
	else
		// Let the parent widget receive the event
		event->ignore ();
}
