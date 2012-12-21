#ifndef UTIL_QPAINTER_H_
#define UTIL_QPAINTER_H_

class QPainter;
class QPoint;
class QPointF;
class QString;

void drawCenteredText (QPainter &painter, const QPoint &position, const QString &text);
void drawCenteredText (QPainter &painter, const QPointF &position, const QString &text);

#endif
