#include "NotificationWidget.h"

#include <iostream>

#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QGridLayout>

#include "src/util/math.h"
#include "src/gui/WidgetFader.h"
#include "src/util/qRectF.h"
#include "src/util/qWidget.h"

// Note that this class uses QPointF and QRectF rather than QPoint and QRect
// due to obscure bottom/right/center calculation rules of QRect.
//
// Possible improvements:
//   * widget showing the time until fade out

// ******************
// ** Construction **
// ******************

NotificationWidget::NotificationWidget (QWidget *parent): QWidget (parent),
	bubbleColor (QColor (0, 0, 0, 191)),
	_topLeftSpacer (new QSpacerItem (0, 0)),
	_rightSpacer   (new QSpacerItem (0, 0)),
	_bottomSpacer  (new QSpacerItem (0, 0)),
	_bubbleLayout (NULL), _contents (NULL),
	_fadeOutDuration (1000), _fadeOutInProgress (false),
	_geometry (this)
{
	// FIXME bubbleLayout and own layout deleted?
	// TODO allow setting the corner radius: set the margins of _bubbleLayout

	// Make label text white and the widget background a rather transparent
	// black (note that this is the widget background, which is not drawn unless
	// the autoFillBackground property is set to true; the bubble is drawn in
	// the bubble color).
	QPalette widgetPalette=palette ();
	widgetPalette.setColor (QPalette::WindowText, Qt::white);
	widgetPalette.setColor (QPalette::Text      , Qt::white);
	widgetPalette.setColor (backgroundRole ()   , QColor (0, 0, 0, 63));
	setPalette (widgetPalette);

	// Add a contents widget - FIXME this should be done from outside, or from a
	// subclass (and _contents should be QWidget *)
	// And make sure to remove the old one from the widget (and take ownership?)
	_contents=new QLabel (this);

	// Setup the layout
	//
	// The widget's layout looks like this:
	// .--------------------------.
	// |top/left| right  spacer   |
	// | spacer | (zero height)   |
	// |--------+-----------------|
	// | bottom | bubble |        |
	// | spacer | layout |        |
	// | (zero  |--------+--------|
	// | width) |        |        |
	// |        |        |        |
	// '--------------------------'
	// The spacers are there to ensure that there is enough space to draw the
	// arrow. Their size depends on the position of the arrow tip, as set by the
	// user. The margins around the layout and the spacing between rows and
	// columns is zero.
	// Note that spacers define the positions of all widget edges relative to
	// the upper left corner of the bubble (a typical layout would define a
	// space between the right/bottom edge of the bubble and the right/bottom
	// edge of the widget). This is required because the arrow tip position does
	// not necessarily depend on the bubble size, but is specified relative to
	// the upper left corner of the bubble.
	// This scheme allows us to draw the arrow on any side of the bubble (though
	// this is not implemented at the moment), but the position of the arrow tip
	// must be specified relative to the top left corner of the bubble. For a
	// different arrow tip position specification, the layout would have to be
	// modified.
	// We also have so set the row column stretch factors because otherwise, if,
	// for example, the bottom spacer is larger than the bubble widget (that is,
	// the arrow points to a location below the bottom of the bubble), the extra
	// space would be distributed between the middle and bottom row, enlarging
	// the bubble. Setting the stretch factor for the middle row and column to 0
	// and the others to 1 ensures that any extra space is always added to the
	// outer columns.
	//
	// The bubble layout is a simple one-element box layout containing the
	// contents widget. Its geometry is identical to the bubble geometry. The
	// margins of the bubble layout are used for specifying the rounded corner
	// radius. That way, the contents widget can never overlap the rounded
	// corner.

	QGridLayout *l=new QGridLayout (this);

	_bubbleLayout=new QHBoxLayout (NULL);
	_bubbleLayout->setContentsMargins (l->contentsMargins ());
	_bubbleLayout->addWidget (_contents);

	l->setMargin (0);
	l->setSpacing (0);
	l->addItem   (_topLeftSpacer, 0, 0);       // Top left
	l->addItem   (_rightSpacer  , 0, 1, 1, 2); // Top middle and right
	l->addItem   (_bottomSpacer , 1, 0, 2, 1); // Left middle and bottom
	l->addLayout (_bubbleLayout , 1, 1);
	l->setColumnStretch (0, 1);
	l->setColumnStretch (1, 0);
	l->setColumnStretch (2, 1);
	l->setRowStretch (0, 1);
	l->setRowStretch (1, 0);
	l->setRowStretch (2, 1);

	// For the default arrow width, we use the corner radius (specifically, the
	// top margin of the bubble layout).
	arrowWidth=_bubbleLayout->contentsMargins ().top ();

	// The geometry will be updated in the resize event before the widget is
	// shown, but we need it now so the widget can be moved before it is shown.
	// FIXME doesn't seem so, can we layout without doing this? We'd like to
	// avoid calling things from the constructor.
	//_geometry.recalculate ();
}

NotificationWidget::~NotificationWidget()
{
}


// **************
// ** Contents **
// **************

QWidget *NotificationWidget::contents ()
{
	return _contents;
}


// *************
// ** Closing **
// *************

/**
 * Emits the closed() event when the widget is closed
 */
void NotificationWidget::closeEvent (QCloseEvent *event)
{
	QWidget::closeEvent (event);
	if (event->isAccepted ())
		emit closed ();
}

/**
 * Sets up a timer that calls fadeOutAndCloseNow after the specified delay (in
 * milliseconds).
 */
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
	_contents->setText (text);
	adjustSize ();
}

QString NotificationWidget::getText () const
{
	return _contents->text ();
}


// ************
// ** Layout **
// ************

/**
 * You must update the geometry after calling this (FIXME better just invalidate
 * it)
 */
void NotificationWidget::updateLayout ()
{
	adjustSize ();

	qDebug () << "Update layout, arrow tip is at" << arrowTipFromBubblePosition;

	double arrowX=arrowTipFromBubblePosition.x ();
	double arrowY=arrowTipFromBubblePosition.y ();

	double top   =ifPositive (-arrowY);
	double left  =ifPositive (-arrowX);
	double bottom=ifPositive (arrowY);
	double right =ifPositive (arrowX);

	qDebug () << "Spacers: top left bottom right is" << top << left << bottom << right;

	_topLeftSpacer->changeSize (left , top   , QSizePolicy::Minimum, QSizePolicy::Minimum);
	_rightSpacer  ->changeSize (right, 0     , QSizePolicy::Minimum, QSizePolicy::Minimum);
	_bottomSpacer ->changeSize (0    , bottom, QSizePolicy::Minimum, QSizePolicy::Minimum);

	// FIXME required?
	layout ()->invalidate ();
	qDebug () << "Geometry: widget" << geometry () << ", bubble:" << _bubbleLayout->geometry ();
}



// **************
// ** Geometry **
// **************

void NotificationWidget::Geometry::recalculate ()
{
	// Determine the geometry of the bubble. It is given by the geometry of the
	// bubble widget.
	bubble=widget->_bubbleLayout->geometry ();

	// Get the bubble margins, which specify the radius of the rounded corners.
	QMargins margins=widget->_bubbleLayout->contentsMargins ();

	// Calculate the corner rectangles
	northWest=northWestCorner (bubble, 2*margins.left  (), 2*margins.top    ());
	northEast=northEastCorner (bubble, 2*margins.right (), 2*margins.top    ());
	southWest=southWestCorner (bubble, 2*margins.left  (), 2*margins.bottom ());
	southEast=southEastCorner (bubble, 2*margins.right (), 2*margins.bottom ());

	// Calculate the arrow coordinates
	// This currently places the arrow immediately below the top left corner.
	// The arrow top is just below the top left corner. The arrow bottom is by
	// arrowWidth lower than the arrow top. The arrow tip position is calculated
	// from its relative position to the bubble.
	arrowTop    = bubble.topLeft () + QPointF (0, margins.top ()    );
	arrowBottom = arrowTop          + QPointF (0, widget->arrowWidth);
	arrowTip    = bubble.topLeft () + widget->arrowTipFromBubblePosition;

	// Create the bubble outline path.
	//
	// Clear the path
	path=QPainterPath ();
	//
	// Draw the path counter-clockwise, starting with the bottom left corner arc
	// and ending after the top left corner arc.
	//
	path.moveTo (southWest.topLeft     ()); path.arcTo (southWest, 180, 90);
	path.lineTo (southEast.bottomLeft  ()); path.arcTo (southEast, 270, 90);
	path.lineTo (northEast.bottomRight ()); path.arcTo (northEast,   0, 90);
	path.lineTo (northWest.topRight    ()); path.arcTo (northWest,  90, 90);
	//
	// Draw the arrow
	path.lineTo (arrowTop);
	path.lineTo (arrowTip);
	path.lineTo (arrowBottom);
	//
	// Close the path
	path.closeSubpath ();
}


// ***********
// ** Shape **
// ***********

QPointF NotificationWidget::defaultBubblePosition (const QPointF &arrowTip)
{
	QMargins margins=_bubbleLayout->contentsMargins ();

	// By default, the arrow tip is placed such that the arrow points straight
	// to the left from its default position right under the top-left corner,
	// and is twice as long as wide.
	QPointF relativeArrowPosition (-2*arrowWidth, margins.top ()+arrowWidth/2);

	return arrowTip-relativeArrowPosition;
}


// **************
// ** Position **
// **************

/**
 * Arguments are in parent coordinates
 *
 * @param arrowTip
 * @param bubblePosition
 */
void NotificationWidget::moveTo (const QPointF &arrowTip, const QPointF &bubblePosition)
{
	// Save the old arrow tip position
	QPointF oldArrowTipFromBubblePosition=arrowTipFromBubblePosition;

	// Calculate the new arrow tip position
	arrowTipFromBubblePosition=arrowTip-bubblePosition;

	// FIXME only if changed (or not initialized yet)
	if (true || arrowTipFromBubblePosition!=oldArrowTipFromBubblePosition)
	{
		updateLayout ();

		// FIXME do automatically after updateLayout by invalidating it.
		// FIXME only if necessary
		// Now we have to recalculate the geometry. This will calculate the
		// position of the bubble and of the arrow tip in widget coordinates.
		_geometry.recalculate ();
	}

	// Finally, we can move the widget. For calculating the position (in parent
	// coordinates), we can use either the arrow tip or the bubble position, the
	// results should be equal. We use the arrow tip position because that is
	// the most important parameter.
	move ((arrowTip-_geometry.arrowTip).toPoint ());
}

void NotificationWidget::moveTo (const QPointF &arrowTip)
{
	moveTo (arrowTip, defaultBubblePosition (arrowTip));
}


// **********
// ** Size **
// **********

void NotificationWidget::resizeEvent (QResizeEvent *event)
{
	(void)event;
	_geometry.recalculate ();
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
	painter.setBrush (bubbleColor);

	painter.drawPath (_geometry.path);
}


// *****************
// ** Interaction **
// *****************

void NotificationWidget::mousePressEvent (QMouseEvent *event)
{
	if (_geometry.path.contains (event->posF ()))
		// The event position is inside the bubble. Act on the event.
		close ();
	else
		// The event position is outside the bubble. Let the parent widget
		// receive the event.
		event->ignore ();
}
