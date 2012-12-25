#ifndef UTIL_QRECTF_H_
#define UTIL_QRECTF_H_

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

QRectF round (const QRectF &rect);
QRectF floor (const QRectF &rect);

QRectF enlarged (const QRectF &rect, double margin);


#endif
