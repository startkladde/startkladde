#ifndef QRECTF_H_
#define QRECTF_H_

#include <QRect>

QRectF northWestCorner (const QRectF &rect, int width, int height);
QRectF northEastCorner (const QRectF &rect, int width, int height);
QRectF southWestCorner (const QRectF &rect, int width, int height);
QRectF southEastCorner (const QRectF &rect, int width, int height);

QRectF northWestCorner (const QRectF &rect, int size);
QRectF northEastCorner (const QRectF &rect, int size);
QRectF southWestCorner (const QRectF &rect, int size);
QRectF southEastCorner (const QRectF &rect, int size);

#endif
