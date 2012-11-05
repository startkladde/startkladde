#include "NotificationWidget.h"

#include <QPainter>
#include <QRect>

NotificationWidget::NotificationWidget (QWidget *parent): QWidget (parent)
{
	ui.setupUi(this);

	// Make label text white
	QPalette widgetPalette=palette ();
	widgetPalette.setColor (QPalette::WindowText, Qt::white);
	setPalette (widgetPalette);
}

NotificationWidget::~NotificationWidget()
{

}

void NotificationWidget::paintEvent (QPaintEvent *event)
{
	(void)event;

	QPainter painter (this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen (Qt::NoPen);

	painter.setBrush (QColor (0, 0, 0, 63));
	painter.drawRect (0, 0, width (), height ());

	painter.setBrush (QColor (0, 0, 0, 191));

	int radius=50;
	int diameter=2*radius;

	int left=20;
	int right=width ();
	int top=0;
	int bottom=height ();

	QRectF nwCorner (left          , top            , diameter, diameter);
	QRectF neCorner (right-diameter, top            , diameter, diameter);
	QRectF swCorner (left          , bottom-diameter, diameter, diameter);
	QRectF seCorner (right-diameter, bottom-diameter, diameter, diameter);

	// We'll draw the path counter-clockwise, starting with the top left corner
	// arc
	QPainterPath path;
	path.moveTo (left +radius, top          ); path.arcTo (nwCorner,  90, 90);
	path.lineTo (left        , bottom-radius); path.arcTo (swCorner, 180, 90);
	path.lineTo (right-radius, bottom       ); path.arcTo (seCorner, 270, 90);
	path.lineTo (right       , top+radius   ); path.arcTo (neCorner,   0, 90);
	path.closeSubpath ();
	painter.drawPath (path);
}
