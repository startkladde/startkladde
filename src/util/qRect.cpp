#include "qRect.h"

#include <QMargins>

// Note that these methods do not use rect.right() and rect.bottom() because,
// other than for QRectF, "the values returned by [these] functions deviate from
// the true bottom-right corner of the rectangle" (see the Qt documentation for
// QRect). The recommended way to handle this problem is to use x()+width() for
// the right edge, and y()+height() for the bottom edge.

QRect northWestCorner (const QRect &rect, int width, int height)
{
	return QRect (rect.x (), rect.y (), width, height);
}

QRect northEastCorner (const QRect &rect, int width, int height)
{
	return QRect (rect.x ()+rect.width ()-width, rect.y (), width, height);
}

QRect southWestCorner (const QRect &rect, int width, int height)
{
	return QRect (rect.x (), rect.y ()+rect.height ()-height, width, height);
}

QRect southEastCorner (const QRect &rect, int width, int height)
{
	return QRect (rect.x ()+rect.width ()-width, rect.y ()+rect.height ()-height, width, height);
}



QRect northWestCorner (const QRect &rect, int size)
{
	return northWestCorner (rect, size, size);
}

QRect northEastCorner (const QRect &rect, int size)
{
	return northEastCorner (rect, size, size);
}

QRect southWestCorner (const QRect &rect, int size)
{
	return southWestCorner (rect, size, size);
}

QRect southEastCorner (const QRect &rect, int size)
{
	return southEastCorner (rect, size, size);
}



QRect northWestCorner (const QRect &rect, const QMargins &margins)
{
	return northWestCorner (rect, 2*margins.left  (), 2*margins.top    ());
}

QRect northEastCorner (const QRect &rect, const QMargins &margins)
{
	return northEastCorner (rect, 2*margins.right (), 2*margins.top    ());
}

QRect southWestCorner (const QRect &rect, const QMargins &margins)
{
	return southWestCorner (rect, 2*margins.left  (), 2*margins.bottom ());
}

QRect southEastCorner (const QRect &rect, const QMargins &margins)
{
	return southEastCorner (rect, 2*margins.right (), 2*margins.bottom ());
}
