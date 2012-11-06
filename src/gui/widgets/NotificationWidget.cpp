#include "NotificationWidget.h"

#include <iostream>

#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QRect>
#include <QDebug>

#include <QTimer>

// ******************
// ** Construction **
// ******************

NotificationWidget::NotificationWidget (QWidget *parent): QWidget (parent),
	cornerRadius (10),
	arrowWidth (10), arrowLength (20),
	backgroundColor (QColor (0, 0, 0, 191)),
	widgetBackgroundColor (QColor (0, 0, 0, 63)),
	drawWidgetBackground (false),
	left (0), right (0), top (0), bottom (0)
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
}

NotificationWidget::~NotificationWidget()
{
}

void NotificationWidget::selfDestruct (int milliseconds)
{
	QTimer::singleShot (milliseconds, this, SLOT (selfDestructNow ()));

}

void NotificationWidget::selfDestructNow ()
{
	QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect (this);
	effect->setOpacity(0.5);
	setGraphicsEffect (effect);
	// QPropertyAnimation for fading out

	QTimer::singleShot (2000, this, SLOT (deleteLater ()));
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

QPoint NotificationWidget::getArrowTip () const
{
//	// FIXME code duplication with paintEvent
//	int y=qMax (arrowPosition, cornerRadius+arrowWidth/2);
//	int x=0;
//	return QPoint (x, y);
	return arrowTip;
}

void NotificationWidget::moveArrowTip (const QPoint &point)
{
	QPoint arrowTip=getArrowTip ();
	move (point-arrowTip);
}

void NotificationWidget::moveArrowTip (int x, int y)
{
	moveArrowTip (QPoint (x, y));
}

void NotificationWidget::resizeEvent (QResizeEvent *event)
{
	(void)event;

	// Calculate the edges of the bubble
	left   = arrowLength;
	right  = width ();
	top    = 0;
	bottom = height ();

	// Calculate the arrow coordinates
	// This currently places the arrow immediately below the top left corner.
	arrowTop   =QPoint (left            , cornerRadius);
	arrowBottom=QPoint (left            , cornerRadius+arrowWidth);
	arrowTip   =QPoint (left-arrowLength, cornerRadius+arrowWidth/2);
}


// **********
// ** Size **
// **********

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

	// FIXME make the rounded corners smaller if the widget is too small
	// Calculate some supplementary values
	int cornerDiameter=2*cornerRadius;

	QRectF nwCorner (left                , top                  , cornerDiameter, cornerDiameter);
	QRectF neCorner (right-cornerDiameter, top                  , cornerDiameter, cornerDiameter);
	QRectF swCorner (left                , bottom-cornerDiameter, cornerDiameter, cornerDiameter);
	QRectF seCorner (right-cornerDiameter, bottom-cornerDiameter, cornerDiameter, cornerDiameter);

	QPainterPath path;
	// Draw the outline counter-clockwise, starting with the bottom left corner
	// arc and ending after the top left corner arc.
	path.moveTo (left              , bottom-cornerRadius); path.arcTo (swCorner, 180, 90);
	path.lineTo (right-cornerRadius, bottom             ); path.arcTo (seCorner, 270, 90);
	path.lineTo (right             , top   +cornerRadius); path.arcTo (neCorner,   0, 90);
	path.lineTo (left +cornerRadius, top                ); path.arcTo (nwCorner,  90, 90);
	// Draw the arrow
	path.lineTo (arrowTop);
	path.lineTo (arrowTip);
	path.lineTo (arrowBottom);
	path.closeSubpath ();
	painter.setBrush (QColor (0, 0, 0, 191));
	painter.drawPath (path);
}

