#include "NotificationWidget.h"

#include <iostream>

#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QRect>
#include <QStyle>

#include "src/util/qRect.h"
#include "src/util/qSize.h"

// If the arrow is somewhere to the left of the widget:
// .-------.--------.
// | arrow | bubble |
// |  tip  |        |
// '-------'--------'
//
// If the arrow is to the left and above the widget:
// .----------------.
// | arrow          |
// |  tip           |
// |       .--------.
// |       | bubble |
// |       |        |
// '-------'--------'
//
// If the arrow is to the left and below the widget:
// .--------.--------.
// |        | bubble |
// |        |        |
// |        '--------'
// | arrow           |
// |  tip            |
// '-----------------'
//
// If the arrow is not to the left, the layout is similar, but the arrow will
// overlap the bubble.

// Note that the arrow tip position is specified relative to the upper left
// corner of the bubble. This scheme allows us to draw the arrow on any side of
// the bubble (though this is not implemented at the moment), but the position
// of the arrow tip must be specified relative to the top left corner of the
// bubble.

// ******************
// ** Construction **
// ******************

NotificationWidget::NotificationWidget (QWidget *parent, Qt::WindowFlags f):
	QWidget (parent, f),
	_contents (NULL), bubbleColor (QColor (0, 0, 0, 191))
{
	// Make label text white and the widget background a rather transparent
	// black (note that this is the widget background, which is not drawn unless
	// the autoFillBackground property is set to true; the bubble is drawn in
	// the bubble color).
	QPalette widgetPalette=palette ();
	widgetPalette.setColor (QPalette::WindowText, Qt::white);
	widgetPalette.setColor (backgroundRole ()   , QColor (0, 0, 0, 63));
	setPalette (widgetPalette);

	_margin=style ()->pixelMetric (QStyle::PM_LayoutTopMargin, 0, this);
	_cornerRadius=_margin;
	_arrowWidth=_margin;
}

NotificationWidget::~NotificationWidget ()
{
	// When setting a contents widget, its parent will be set to this, so it
	// will be deleted automatically.
}


// **************
// ** Contents **
// **************

void NotificationWidget::setContents (QWidget *contents)
{
	if (_contents)
	{
		// Delete the old contents widget
		_contents->deleteLater ();
	}

	// Set the new contents widget
	_contents=contents;

	if (_contents)
	{
		// Reparent the new widget to this - it will be deleted in the
		// destructor. This also makes the contents widget invisible.
		_contents->setParent (this);
		_contents->show ();
	}

	invalidate ();
}

/**
  * Note that this will work even if the contents widget is a QLabel set by the
  * user, for example, a QLabel subclass.
  */
void NotificationWidget::setText (const QString &text)
{
	// If the contents widget is not a label, make it one
	QLabel *label=dynamic_cast<QLabel *> (_contents);
	if (!label)
	{
		label=new QLabel (this);

		QPalette labelPalette=label->palette ();
		labelPalette.setColor (QPalette::WindowText, Qt::white);
		labelPalette.setColor (QPalette::Text, Qt::white);
		label->setPalette (labelPalette);

		setContents (label);
	}

	label->setText (text);
}

QString NotificationWidget::text () const
{
	QLabel *label=dynamic_cast<QLabel *> (_contents);
	if (label)
		return label->text ();
	else
		return QString ();
}


// ****************
// ** Parameters **
// ****************

void NotificationWidget::setArrowWidth (int arrowWidth)
{
	_arrowWidth=arrowWidth;
	invalidate ();
}

void NotificationWidget::setCornerRadius (int cornerRadius)
{
	_cornerRadius=cornerRadius;
	invalidate ();
}

void NotificationWidget::setMargin (int margin)
{
	_margin=margin;
	invalidate ();
}


// **************
// ** Position **
// **************

QPoint NotificationWidget::defaultBubblePosition (const QPoint &arrowTip) const
{
	// By default, the arrow tip is placed such that the arrow points straight
	// to the left from its default position right under the top-left corner,
	// and is twice as long as wide.
	int arrowX = -2 *_arrowWidth;
	int arrowY = _cornerRadius + _arrowWidth/2;

	QPoint relativeArrowPosition (arrowX, arrowY);
	return arrowTip - relativeArrowPosition;
}

/** Arguments are in parent coordinates */
void NotificationWidget::moveTo (const QPoint &arrowTip, const QPoint &bubblePosition)
{
	_arrowTipFromBubblePosition=arrowTip-bubblePosition;
	invalidate ();

	// Move the widget. For calculating the position (in parent coordinates), we
	// can use either the arrow tip or the bubble position, the results should
	// be equal. We use the arrow tip position because it is more important.
	move (arrowTip - this->arrowTip ());
}

void NotificationWidget::moveTo (const QPoint &arrowTip)
{
	moveTo (arrowTip, defaultBubblePosition (arrowTip));
}


// ************
// ** Layout **
// ************

QSize NotificationWidget::bubbleSizeHint () const
{
	// Determine the space we need for the contents (may be zero if we have no
	// contents).
	QSize contentsSizeHint;
	if (_contents)
		contentsSizeHint=_contents->sizeHint ();

	// Add the margin around the contents
	QSize bubbleSize=contentsSizeHint+QSize (2*_margin, 2*_margin);

	// Enlarge the bubble to the minimum bubble size, if necessary
	bubbleSize=max (bubbleSize, minimumBubbleSize ());

	return bubbleSize;
}

QSize NotificationWidget::sizeHint () const
{
	// Determine the bubble size
	QSize bubbleSize=bubbleSizeHint ();

	// Extend the bubble size to include the arrow tip
	int width =qMax (bubbleSize.width  (), right  ())+left ();
	int height=qMax (bubbleSize.height (), bottom ())+top  ();

	return QSize (width, height);
}

// Invalidates cached information and posts a layout request
void NotificationWidget::invalidate ()
{
	// Notify the parent that our size hint (may have) changed
	updateGeometry ();

	// Schedule an update of our own layout, potentially resizing the widget
	QApplication::postEvent (this, new QEvent (QEvent::LayoutRequest));
}

// Called when a layout request is received
void NotificationWidget::doLayout ()
{
	if (_contents)
	{
		QSize contentsSize=_contents->sizeHint ();
		QPoint contentsPosition=centeredIn (contentsSize, bubbleGeometry ());

		_contents->setGeometry (QRect (contentsPosition, contentsSize));
	}

	// Clear the path
	_path_=QPainterPath ();

	// Note that even if the contents' size changed, the widget's size does not
	// necessarily change: if the arrow tip is to the bottom and/or right of the
	// bubble, the widget's size is defined by the arrow tip position. In this
	// case, we won't get a resize event, but we still have to make sure that
	// the widget is repainted.
	update ();
}


// **************
// ** Geometry **
// **************

QPainterPath NotificationWidget::path ()
{
	if (_path_.isEmpty ())
	{
		QPoint arrowTip=this->arrowTip ();

		QRect bubble=bubbleGeometry ();

		// Calculate the corner rectangles
		// We use QRectF here in order to be able to use the bottom/right methods,
		// which are not usable for QRect.
		QRectF northWest=northWestCorner (bubble, 2*_cornerRadius);
		QRectF northEast=northEastCorner (bubble, 2*_cornerRadius);
		QRectF southWest=southWestCorner (bubble, 2*_cornerRadius);
		QRectF southEast=southEastCorner (bubble, 2*_cornerRadius);

		// Calculate the arrow base coordinates
		// This tries to place the arrow base on the same height as the tip to make
		// it as straight as possible, if that is allowed by the bubble dimensions.
		// If not, it will be right below the top left corner or right above the
		// bottom left corner of the bubble.
		int arrowCenter=qBound (
			bubble.top ()                 +_cornerRadius+_arrowWidth/2,
			arrowTip.y (),
			bubble.top ()+bubble.height ()-_cornerRadius-_arrowWidth/2);

		QPoint arrowTop    (bubble.left (), arrowCenter-_arrowWidth/2);
		QPoint arrowBottom (bubble.left (), arrowCenter+_arrowWidth/2);


		// Clear the path
		_path_=QPainterPath ();

		// Draw the path counter-clockwise, starting with the bottom left corner arc
		// and ending after the top left corner arc.
		_path_.moveTo (southWest.topLeft     ().toPoint ()); _path_.arcTo (southWest, 180, 90);
		_path_.lineTo (southEast.bottomLeft  ().toPoint ()); _path_.arcTo (southEast, 270, 90);
		_path_.lineTo (northEast.bottomRight ().toPoint ()); _path_.arcTo (northEast,   0, 90);
		_path_.lineTo (northWest.topRight    ().toPoint ()); _path_.arcTo (northWest,  90, 90);
		//
		// Draw the arrow
		_path_.lineTo (arrowTop);
		_path_.lineTo (arrowTip);
		_path_.lineTo (arrowBottom);
		//
		// Close the path
		_path_.closeSubpath ();
	}

	return _path_;
}


// ************
// ** Events **
// ************

bool NotificationWidget::event (QEvent *e)
{
	if (e->type ()==QEvent::LayoutRequest)
	{
		layoutRequestEvent ();
		return true;
	}

	return QWidget::event (e);
}

void NotificationWidget::closeEvent (QCloseEvent  *e)
{
	(void)e;
	emit closed ();
}

void NotificationWidget::layoutRequestEvent ()
{
	// Make sure that the widget is at the correct size to hold the contents
	// Do this here rather than in doLayout so we can call doLayout from
	// resizeEvent without the risk of causing recursion.
	setFixedSize (sizeHint ());
	doLayout ();
}

void NotificationWidget::mousePressEvent (QMouseEvent *e)
{
	// If the event position is outside the bubble, let the parent widget
	// receive the event.
	if (!path ().contains (e->pos ()))
	{
		e->ignore ();
		return;
	}

	// React to the event
	emit clicked ();
}

void NotificationWidget::paintEvent (QPaintEvent *e)
{
    (void)e;

	QWidget::paintEvent (e);

	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing);
	painter.setPen (Qt::NoPen);
	painter.setBrush (bubbleColor);
	painter.drawPath (path ());
}
