#include "src/util/qPainter.h"

#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QString>

#include "src/util/qRect.h"
#include "src/util/qRectF.h"

void drawCenteredText (QPainter &painter, const QPoint &position, const QString &text)
{
	QSize size=painter.fontMetrics ().size (0, text);
	QRect rect=centeredQRect (position, size);
	painter.fillRect (rect, painter.brush ());
	painter.drawText (rect, Qt::AlignHCenter, text);
}

void drawCenteredText (QPainter &painter, const QPointF &position, const QString &text)
{
	QSize size=painter.fontMetrics ().size (0, text);
	QRectF rect=centeredQRectF (position, QSizeF (size));
	painter.fillRect (rect, painter.brush ());
	painter.drawText (rect, text);
}

