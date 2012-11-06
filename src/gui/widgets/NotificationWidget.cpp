#include "NotificationWidget.h"

#include <QPainter>
#include <QRect>
#include <QDebug>

NotificationWidget::NotificationWidget (QWidget *parent): QWidget (parent),
	cornerRadius (10),
	arrowWidth (3*10), arrowLength (20), arrowPosition (30),
	backgroundColor (QColor (0, 0, 0, 191)),
	widgetBackgroundColor (QColor (0, 0, 0, 63)),
	drawWidgetBackground (false)
{
	ui.setupUi (this);

	// Use the default left layout margin as corner radius
//	cornerRadius=ui.widgetLayout->contentsMargins ().left ();

	// Set the layout margins to the corner radius. The left margin also
	// contains the arrow.
	ui.widgetLayout->setContentsMargins (
		arrowLength+cornerRadius,
		cornerRadius,
		cornerRadius,
		cornerRadius);

	// Make label text white
	QPalette widgetPalette=palette ();
	widgetPalette.setColor (QPalette::WindowText, Qt::white);
	setPalette (widgetPalette);

//	setMinimumSize (QSize (10, 10));

	// Make the widget as compact as possible
	resize (minimumSizeHint ());
}

NotificationWidget::~NotificationWidget()
{

}

void NotificationWidget::setDrawWidgetBackground (bool drawWidgetBackground)
{
	this->drawWidgetBackground=drawWidgetBackground;
}

bool NotificationWidget::getDrawWidgetBackground () const
{
	return drawWidgetBackground;
}

QPoint NotificationWidget::getArrowTip () const
{
	// FIXME code duplication with paintEvent
	int y=qMax (arrowPosition, cornerRadius+arrowWidth/2);
	int x=0;
	return QPoint (x, y);
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

	// Calculate the edges of the bubble
	int left   = arrowLength;
	int right  = width ();
	int top    = 0;
	int bottom = height ();

	// FIXME make the rounded corners smaller if the widget is too small
	// Calculate some supplementary values
	int cornerDiameter=2*cornerRadius;

	// Calculate some arrow coordinates
	// The effective arrow position must be at least radius+arrowWidth/2,
	// otherwise the arrow will overlap with the upper left corner.
	// FIXME make sure it doesn't overlap with the lower left corner - don't
	// draw the arrow if the widget is too small
	int effectiveArrowPosition=qMax (arrowPosition, cornerRadius+arrowWidth/2);
	QPoint arrowTop    (left            , effectiveArrowPosition-arrowWidth/2);
	QPoint arrowTip    (left-arrowLength, effectiveArrowPosition);
	QPoint arrowBottom (left            , effectiveArrowPosition+arrowWidth/2);

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

QSize NotificationWidget::minimumSizeHint () const
{
	QSize hint=QWidget::minimumSizeHint ();

	// Make sure the widget is large enough for two of the rounded corners
	hint.setHeight (qMax (hint.height (), 2*cornerRadius));
	hint.setWidth  (qMax (hint.width  (), 2*cornerRadius));

	// Make sure the widget is large enough for the arrow and the lower corner
	hint.setHeight (qMax (hint.height (), arrowPosition+arrowWidth/2+cornerRadius));

	qDebug () << hint;

	return hint;
}
