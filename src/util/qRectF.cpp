#include "qRectF.h"

#include <cmath>

#include <QMargins>

QRectF northWestCorner (const QRectF &rect, int width, int height)
{
	return QRectF (rect.left (), rect.top (), width, height);
}

QRectF northEastCorner (const QRectF &rect, int width, int height)
{
	return QRectF (rect.right ()-width, rect.top (), width, height);
}

QRectF southWestCorner (const QRectF &rect, int width, int height)
{
	return QRectF (rect.left (), rect.bottom ()-height, width, height);
}

QRectF southEastCorner (const QRectF &rect, int width, int height)
{
	return QRectF (rect.right ()-width, rect.bottom ()-height, width, height);
}



QRectF northWestCorner (const QRectF &rect, int size)
{
	return northWestCorner (rect, size, size);
}

QRectF northEastCorner (const QRectF &rect, int size)
{
	return northEastCorner (rect, size, size);
}

QRectF southWestCorner (const QRectF &rect, int size)
{
	return southWestCorner (rect, size, size);
}

QRectF southEastCorner (const QRectF &rect, int size)
{
	return southEastCorner (rect, size, size);
}



QRectF northWestCorner (const QRectF &rect, const QMargins &margins)
{
	return northWestCorner (rect, 2*margins.left  (), 2*margins.top    ());
}

QRectF northEastCorner (const QRectF &rect, const QMargins &margins)
{
	return northEastCorner (rect, 2*margins.right (), 2*margins.top    ());
}

QRectF southWestCorner (const QRectF &rect, const QMargins &margins)
{
	return southWestCorner (rect, 2*margins.left  (), 2*margins.bottom ());
}

QRectF southEastCorner (const QRectF &rect, const QMargins &margins)
{
	return southEastCorner (rect, 2*margins.right (), 2*margins.bottom ());
}



QRectF centeredQRectF (const QPointF &point, double size)
{
	return QRectF (point.x ()-size/2, point.y ()-size/2, size, size);
}

QRectF centeredQRectF (const QPointF &point, const QSizeF &size)
{
	double x=point.x ();
	double y=point.y ();
	double w=size.width ();
	double h=size.height ();

	return QRectF (x-w/2, y-h/2, w, h);
}

QRectF round (const QRectF &rect)
{
	double left  =round (rect.left   ()+0.01);
	double top   =round (rect.top    ()+0.01);
	double width =round (rect.width  ()+0.01);
	double height=round (rect.height ()+0.01);

	return QRectF (left, top, width, height);
}

QRectF floor (const QRectF &rect)
{
	double left  =floor (rect.left   ());
	double top   =floor (rect.top    ());
	double width =floor (rect.width  ());
	double height=floor (rect.height ());

	return QRectF (left, top, width, height);
}


QRectF enlarged (const QRectF &rect, double margin)
{
	return QRectF (
		rect.left ()-margin,
		rect.top  ()-margin,
		rect.width  ()+2*margin,
		rect.height ()+2*margin);
}

/**
 * Calculates the minimum distance from the specified point to any point in the
 * specified rectangle
 */
double minimumDistance (const QRectF &rect, const QPointF &point)
{
	// Get the relevant coordinates of the point and the rectangle. Note that
	// the coordinate system has y down, so bottom>=top.
	double x=point.x ();
	double y=point.y ();
	double left=rect.left ();
	double right=rect.right ();
	double top=rect.top ();
	double bottom=rect.bottom ();

	double dx, dy;

	if      (x<left ) dx=left-x;  // The point is left of the rectangle
	else if (x>right) dx=x-right; // The point is right of the rectangle
	else              dx=0;       // The point is horizontally within the rectangle

	if      (y<top)    dy=top-y;    // The point is above the rectangle
	else if (y>bottom) dy=y-bottom; // The point is below the rectangle
	else               dy=0;        // The point is vertically within the rectangle

	return sqrt (dx*dx+dy*dy);
}

/**
 * Calculates the minimum distance from the specified point to any point in the
 * specified rectangle
 */
double maximumDistance (const QRectF &rect, const QPointF &point)
{
	// The distance is always the distance to a corner

	// Get the relevant coordinates of the point and the rectangle. Note that
	// the coordinate system has y down, so bottom>=top.
	double x=point.x ();
	double y=point.y ();
	double left=rect.left ();
	double right=rect.right ();
	double top=rect.top ();
	double bottom=rect.bottom ();
	QPointF center=rect.center ();

	double dx, dy;

	if (x>=center.x ()) dx=x-left;
	else                dx=right-x;

	if (y>=center.y ()) dy=y-top;
	else                dy=bottom-y;

	return sqrt (dx*dx+dy*dy);
}
