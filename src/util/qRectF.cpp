#include "qRectF.h"

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
