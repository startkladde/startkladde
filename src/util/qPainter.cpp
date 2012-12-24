#include "src/util/qPainter.h"

#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QString>

#include "src/util/qRect.h"
#include "src/util/qRectF.h"

QSize textSize (const QPainter &painter, const QString &text)
{
	return painter.fontMetrics ().size (0, text);
}

void drawCenteredText (QPainter &painter, const QPoint &position, const QString &text)
{
	QSize size=textSize (painter, text);
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

void drawOrthogonalCross (QPainter &painter, const QPoint &position, int size)
{
	QPoint dx (size, 0);
	QPoint dy (0, size);
	painter.drawLine (position-dx, position+dx);
	painter.drawLine (position-dy, position+dy);
}

void drawOrthogonalCross (QPainter &painter, const QPointF &position, double size)
{
	QPointF dx (size, 0);
	QPointF dy (0, size);
	painter.drawLine (position-dx, position+dx);
	painter.drawLine (position-dy, position+dy);
}

void drawDiagonalCross (QPainter &painter, const QPoint &position, int size)
{
	QPoint dx (size, size);
	QPoint dy (size, -size);
	painter.drawLine (position-dx, position+dx);
	painter.drawLine (position-dy, position+dy);
}

void drawDiagonalCross (QPainter &painter, const QPointF &position, double size)
{
	QPointF dx (size, size);
	QPointF dy (size, -size);
	painter.drawLine (position-dx, position+dx);
	painter.drawLine (position-dy, position+dy);
}
