#ifndef UTIL_QRECTF_H_
#define UTIL_QRECTF_H_

#include <QLineF>
#include <QRect>

class QMargins;

QRectF northWestCorner (const QRectF &rect, int width, int height);
QRectF northEastCorner (const QRectF &rect, int width, int height);
QRectF southWestCorner (const QRectF &rect, int width, int height);
QRectF southEastCorner (const QRectF &rect, int width, int height);

QRectF northWestCorner (const QRectF &rect, int size);
QRectF northEastCorner (const QRectF &rect, int size);
QRectF southWestCorner (const QRectF &rect, int size);
QRectF southEastCorner (const QRectF &rect, int size);

QRectF northWestCorner (const QRectF &rect, const QMargins &margins);
QRectF northEastCorner (const QRectF &rect, const QMargins &margins);
QRectF southWestCorner (const QRectF &rect, const QMargins &margins);
QRectF southEastCorner (const QRectF &rect, const QMargins &margins);

QRectF centeredQRectF (const QPointF &point, double size);
QRectF centeredQRectF (const QPointF &point, const QSizeF &size);

QRectF alignedQRectF (const QPointF &point, Qt::Alignment alignment, double size);
QRectF alignedQRectF (const QPointF &point, Qt::Alignment alignment, const QSizeF &size);

QRectF round (const QRectF &rect);
QRectF floor (const QRectF &rect);

QRectF enlarged (const QRectF &rect, double margin);

double minimumDistance (const QRectF &rect, const QPointF &point);
double maximumDistance (const QRectF &rect, const QPointF &point);

QLineF topLine    (const QRectF &rect);
QLineF bottomLine (const QRectF &rect);
QLineF leftLine   (const QRectF &rect);
QLineF rightLine  (const QRectF &rect);

#endif
