#ifndef UTIL_QPAINTER_H_
#define UTIL_QPAINTER_H_

#include <QSize>

class QPainter;
class QPoint;
class QPointF;
class QString;

QSize textSize (const QPainter &painter, const QString &text);
void drawCenteredText (QPainter &painter, const QPoint  &position, const QString &text);
void drawCenteredText (QPainter &painter, const QPointF &position, const QString &text);
void drawOrthogonalCross (QPainter &painter, const QPoint  &position, int    size);
void drawOrthogonalCross (QPainter &painter, const QPointF &position, double size);
void drawDiagonalCross (QPainter &painter, const QPoint  &position, int    size);
void drawDiagonalCross (QPainter &painter, const QPointF &position, double size);

#endif
