#include <src/util/qPointF.h>

#include <QPointF>

double min (const QPointF &point)
{
	return qMin (point.x (), point.y ());
}

double max (const QPointF &point)
{
	return qMax (point.x (), point.y ());
}
