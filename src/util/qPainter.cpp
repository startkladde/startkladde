#include "src/util/qPainter.h"

#include <iostream>
#include <cmath>

#include <QDebug>
#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QString>

#include "src/util/qRect.h"
#include "src/util/qRectF.h"


// ************
// ** Shapes **
// ************

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
	size=size/M_SQRT2;
	QPoint dx (size, size);
	QPoint dy (size, -size);
	painter.drawLine (position-dx, position+dx);
	painter.drawLine (position-dy, position+dy);
}

void drawDiagonalCross (QPainter &painter, const QPointF &position, double size)
{
	size=size/M_SQRT2;
	QPointF dx (size, size);
	QPointF dy (size, -size);
	painter.drawLine (position-dx, position+dx);
	painter.drawLine (position-dy, position+dy);
}

void drawCircle (QPainter &painter, const QPoint &center, int radius)
{
	painter.drawArc (centeredQRect (center, radius), 0, 16*360);
}

void drawCircle (QPainter &painter, const QPointF &center, double radius)
{
	painter.drawArc (centeredQRectF (center, 2*radius), 0, 16*360);
}


// **********
// ** Text **
// **********

QSize textSize (const QPainter &painter, const QString &text)
{
	return painter.fontMetrics ().size (0, text);
}

void drawCenteredText (QPainter &painter, const QPoint &position, const QString &text, int margin)
{
	// FIXME call other drawCenteredText instead?
	QSize size=textSize (painter, text);
	QRect rect=centeredQRect (position, size);
	painter.fillRect (enlarged (rect, margin), painter.brush ());
	painter.drawText (rect, Qt::AlignHCenter, text);
}

void drawCenteredText (QPainter &painter, const QPointF &position, const QString &text, double margin)
{
	QSize size=textSize (painter, text);
	QRectF rect=centeredQRectF (position, QSizeF (size));
	// It seems like drawText, while it accepts a QRectF, is not able to draw to
	// subpixel positions. This causes jumping text due to rounding issues. In
	// order to at least keep text and background rectangle coincident, we round
	// the position manually.
	rect=round (rect);
	painter.fillRect (enlarged (rect, margin), painter.brush ());
	painter.drawText (rect, text); // FIXME need Qt::AlignHCenter?
}

void drawText (QPainter &painter, const QPoint &position, Qt::Alignment alignment, const QString &text, int margin)
{
	return drawText (painter, QPointF (position), alignment, text, margin);
}

void drawText (QPainter &painter, const QPointF &position, Qt::Alignment alignment, const QString &text, double margin)
{
	QSize size=textSize (painter, text);
	QRectF rect=alignedQRectF (position, alignment, QSizeF (size));
	// It seems like drawText, while it accepts a QRectF, is not able to draw to
	// subpixel positions. This causes jumping text due to rounding issues. In
	// order to at least keep text and background rectangle coincident, we round
	// the position manually.
	rect=round (rect);
	painter.fillRect (enlarged (rect, margin), painter.brush ());
	painter.drawText (rect, text);
}
