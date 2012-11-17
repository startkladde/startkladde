#ifndef QRECTF_H_
#define QRECTF_H_

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

#endif
