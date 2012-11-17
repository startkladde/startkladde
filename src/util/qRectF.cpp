#include "qRectF.h"

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
