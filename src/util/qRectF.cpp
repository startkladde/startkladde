#include "qRectF.h"

QRectF northWestCorner (const QRectF &rect, int size)
{
	return QRectF (rect.left (), rect.top (), size, size);
}

QRectF northEastCorner (const QRectF &rect, int size)
{
	return QRectF (rect.right ()-size, rect.top (), size, size);
}

QRectF southWestCorner (const QRectF &rect, int size)
{
	return QRectF (rect.left (), rect.bottom ()-size, size, size);
}

QRectF southEastCorner (const QRectF &rect, int size)
{
	return QRectF (rect.right ()-size, rect.bottom ()-size, size, size);
}
