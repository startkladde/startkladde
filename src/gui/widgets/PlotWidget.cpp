#include "PlotWidget.h"

#include <cmath>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

// Notes:
//   * whenever the plot coordinate system changes, call invalidateView
//   * before accessing the transforms, call updateTransforms

PlotWidget::PlotWidget (QWidget *parent): QFrame (parent)
{
	// Default orientation: plot coordinate system origin at widget center,
	// smaller side 1, x axis right
	_center_p=QPointF (0, 0);
	_diameter_p=1;
	_orientation=Angle::zero ();

	// The transforms are invalid
	_transformsValid=false;

	// None of the mouse actions are active
	_mouseScrollActive=false;
	_mouseZoomActive=false;
	_mouseRotationActive=false;

	// The zoom factors: the magnitude of action to zoom in by a factor of two
	// Mouse wheel down (back) means zooming out. This is the convention that
	// many other applications, including Firefox and Gimp, use.
	_mouseZoomDoubleDistance_w=50;
	_mouseWheelZoomDoubleAngle=Angle::fromDegrees (120);
	_keyboardZoomDoubleCount=8;

	// The rotation factors: the magnitude of action to rotate for a full
	// revolution
	_mouseRotationRevolutionDistance_w=-360; // 1° per pixel
	_keyboardRotationRevolutionCount=36; // 10° per keypress

	// For the mouse actions, the initial mouse positions are irrelevant as they
	// are set when the actions starts and ignored as long as the action is not
	// active, and so are the original values of the respective properties (they
	// are still initialized for good coding practice).
	_mouseZoomStartPosition_w=QPoint (0, 0);
	_mouseRotationStartPosition_w=QPoint (0, 0);
	//
	_mouseScrollPosition_p=QPointF (0, 0);
	_mouseZoomOriginalDiameter_p=1;
	_mouseRotationOriginalOrientation=Angle::zero ();
}

PlotWidget::~PlotWidget ()
{
}


// **************
// ** Position **
// **************

QPointF PlotWidget::center_p () const
{
	return _center_p;
}

void PlotWidget::setCenter_p (const QPointF &center_p)
{
	_center_p=center_p;
	invalidateView ();
}

void PlotWidget::scrollToCenter (const QPointF &position_p)
{
	setCenter_p (position_p);
}

void PlotWidget::scrollTo (const QPointF &position_p, const QPointF &position_w)
{
	// We want the plot position position_p to be at the target position
	// position_w. We therefore calculate the location that is currently at this
	// position and correct the center location by this difference. We do  this
	// in plot coordinates because this is the coordinate system the center
	// location is stored in.
	QPointF currentPosition_p=toPlot (position_w);
	setCenter_p (center_p ()+position_p-currentPosition_p);
}

void PlotWidget::scrollBy (double dx_w, double dy_w)
{
	scrollTo (toPlot (QPointF (dx_w, dy_w)), QPointF (0, 0));
}


// *****************
// ** Orientation **
// *****************

void PlotWidget::setOrientation (const Angle &orientation)
{
	// Normalize the new orientation
	Angle normalizedOrientation=orientation.normalized ();

	// If the value is already current, stop
	if (_orientation==normalizedOrientation)
		return;

	// Assign the (normalized) new value
	_orientation=normalizedOrientation;

	// Invalidate cached data
	invalidateView ();

	// Emit the orientationChanged signal, *after* setting the new value and
	// invalidating cached data
	emit orientationChanged ();
}

Angle PlotWidget::orientation () const
{
	return _orientation;
}

void PlotWidget::rotateBy (const Angle &rotation)
{
	setOrientation (_orientation+rotation);
}


// ***********
// ** Scale **
// ***********

double PlotWidget::diameter_p () const
{
	return _diameter_p;
}

void PlotWidget::setDiameter_p (double diameter_p)
{
	_diameter_p=diameter_p;
	invalidateView ();
}

QSizeF PlotWidget::size_p () const
{
	double widgetAspectRatio = width () / (double)height ();

	if (widgetAspectRatio>=1)
		// The widget is wider than high
		return QSizeF (_diameter_p*widgetAspectRatio, _diameter_p);
	else
		// The widget is higher than wide
		return QSizeF (_diameter_p, _diameter_p/widgetAspectRatio);
}

void PlotWidget::zoomInBy (double factor)
{
	setDiameter_p (_diameter_p/factor);
}

/**
 * Calculates the length, in plot coordinates, of one widget coordinate length
 * unit (one pixel)
 */
double PlotWidget::widgetScale_p () const
{
	return _diameter_p/qMin (width (), height ());
}

/**
 * Calculates the length, in widget coordinates (pixels), of one plot coordinate
 * length unit
 */
double PlotWidget::plotScale_w () const
{
	return 1/widgetScale_p ();
}


// ***********************
// ** Coordinate system **
// ***********************

void PlotWidget::invalidateView ()
{
	// Mark the transforms as invalid. They will be recalculated the next time
	// they are needed. The viewChanged signal will also be emitted at that
	// point.
	_transformsValid=false;

	// Schedule a repaint
	update ();
}

void PlotWidget::updateTransforms () const
{
	if (_transformsValid)
		return;

	//qDebug () << "Recalculating transforms with " << _center_p << _orientation << _diameter_p;

	// _w_T_p is supposed to describe the widget coordinate system in plot
	// coordinates. This is achieved by transforming in several steps. The
	// transforms always apply to the current coordinate system.
	//
	// Start at the plot coordinate system itself
	_w_T_p=QTransform ();
	// Translate it so that its origin is at the center of the widget
	_w_T_p.translate (_center_p.x (), _center_p.y ());
	// Rotate it so that the x axis is parallel to the widget's x axis; this
	// will point the y axis opposite to the widget's y axis.
	_w_T_p.rotateRadians (-_orientation.toRadians ());
	// Flip it upside down so both axes are parallel to the widget's axes
	_w_T_p.scale (1, -1);
	// Scale it so that the diameter fits
	double scale=_diameter_p/qMin (width (), height ());
	_w_T_p.scale (scale, scale);
	// Translate it to the origin of the widget coordinate system
	_w_T_p.translate (-width ()/2.0, -height ()/2.0);

	// Calculate the inverse transform
	_p_T_w=_w_T_p.inverted ();

	_transformsValid=true;

	emit viewChanged ();
}


// *********************
// ** Qt mouse events **
// *********************

void PlotWidget::mousePressEvent (QMouseEvent *event)
{
	if (event->button ()==Qt::LeftButton)
	{
		_mouseScrollPosition_p=toPlot (event->posF ());
		_mouseScrollActive=true;
		event->accept ();
	}
	else if (event->button ()==Qt::MiddleButton)
	{
		_mouseZoomStartPosition_w=event->pos ();
		_mouseZoomOriginalDiameter_p=diameter_p ();
		_mouseZoomActive=true;

		_mouseRotationStartPosition_w=event->pos ();
		_mouseRotationOriginalOrientation=orientation ();
		_mouseRotationActive=true;
		event->accept ();
	}
	else
	{
		QFrame::mousePressEvent (event);
	}
}

void PlotWidget::mouseReleaseEvent (QMouseEvent *event)
{
	if (event->button ()==Qt::LeftButton)
	{
		_mouseScrollActive=false;
		event->accept ();
	}
	else if (event->button ()==Qt::MiddleButton)
	{
		_mouseZoomActive=false;
		_mouseRotationActive=false;
		event->accept ();
	}
}

void PlotWidget::mouseMoveEvent (QMouseEvent *event)
{
	emit mouseMoved_p (toPlot (event->posF ()));

	if (_mouseScrollActive)
	{
		scrollTo (_mouseScrollPosition_p, event->posF ());
	}

	if (_mouseZoomActive)
	{
		// TODO zoom around the initial mouse position
		int deltaY=event->pos ().y () - _mouseZoomStartPosition_w.y ();
		setDiameter_p (_mouseZoomOriginalDiameter_p*pow (2, deltaY/_mouseZoomDoubleDistance_w));
	}

	if (_mouseRotationActive)
	{
		int deltaX=event->pos ().x () - _mouseRotationStartPosition_w.x ();
		Angle deltaAngle=Angle::fullCircle ()*deltaX/_mouseRotationRevolutionDistance_w;
		setOrientation (_mouseRotationOriginalOrientation+deltaAngle);
	}
}

void PlotWidget::wheelEvent (QWheelEvent *event)
{
	// Store the previous position so we can zoom around the mouse position
	QPointF position_w=QPointF (event->pos ());
	QPointF position_p=toPlot (position_w);

	// TODO zoom around the mouse wheel position
	Angle angle=Angle::fromDegrees (event->delta ()/(double)8);
	zoomInBy (pow (2, angle/_mouseWheelZoomDoubleAngle));

	// Scroll so the same point as before is at the mouse position
	scrollTo (position_p, position_w);
}


void PlotWidget::leaveEvent (QEvent *event)
{
	(void)event;
	emit mouseLeft ();
}


// ************************
// ** Qt keyboard events **
// ************************

void PlotWidget::keyPressEvent (QKeyEvent *event)
{
	// Note that we don't call event->accept (). The documentation for QWidget::
	// keyPressEvent recommends that implementations do not call the superclass
	// method if they act upon the key.
	const double keyboardZoomFactor=pow (2, 1/(double)_keyboardZoomDoubleCount);
	const Angle keyboardRotation=Angle::fullCircle ()/_keyboardRotationRevolutionCount;
	const int keyboardScrollDistance=0.1*qMin (width (), height ());

	switch (event->key ())
	{
		// Zoom in. The Key_Equal is a common hack for english keyboard layouts
		// where Key_Plus is on the same key, but shifted.
		case Qt::Key_Plus:         zoomInBy (keyboardZoomFactor); break;
		case Qt::Key_Equal:        zoomInBy (keyboardZoomFactor); break;

		// Zoom out
		case Qt::Key_Minus:        zoomInBy (1/keyboardZoomFactor); break;

		// Rotate
		case Qt::Key_BracketLeft:  rotateBy ( keyboardRotation); break;
		case Qt::Key_BracketRight: rotateBy (-keyboardRotation); break;

		// Scroll
		case Qt::Key_Right: case Qt::Key_L: scrollBy ( keyboardScrollDistance, 0); break;
		case Qt::Key_Left : case Qt::Key_H: scrollBy (-keyboardScrollDistance, 0); break;
		case Qt::Key_Up   : case Qt::Key_K: scrollBy (0, -keyboardScrollDistance); break;
		case Qt::Key_Down : case Qt::Key_J: scrollBy (0,  keyboardScrollDistance); break;

		// Other
		default: QFrame::keyPressEvent (event); break;
	}
}


// *********************
// ** Other Qt events **
// *********************

void PlotWidget::resizeEvent (QResizeEvent *event)
{
	(void)event;
	invalidateView ();
}


// ****************
// ** Transforms **
// ****************

QPointF PlotWidget::toWidget (const QPointF &point_p) const
{
	updateTransforms ();
	return point_p * _p_T_w;
}

QPointF PlotWidget::toPlot (const QPointF &point_w) const
{
	updateTransforms ();
	return point_w * _w_T_p;
}

QPointF PlotWidget::toWidget (double x_p, double y_p) const
{
	return toWidget (QPointF (x_p, y_p));
}

QPointF PlotWidget::toPlot (double x_w, double y_w) const
{
	return toPlot (QPointF (x_w, y_w));
}

QPolygonF PlotWidget::toWidget (const QPolygonF &Polygon_p) const
{
	updateTransforms ();
	return Polygon_p * _p_T_w;
}

QPolygonF PlotWidget::toPlot (const QPolygonF &Polygon_w) const
{
	updateTransforms ();
	return Polygon_w * _w_T_p;
}

double PlotWidget::toWidget (double length_p) const
{
	updateTransforms ();
	return length_p*plotScale_w ();
}

double PlotWidget::toPlot (double length_w) const
{
	updateTransforms ();
	return length_w*widgetScale_p ();
}

void PlotWidget::transformToPlot (QPainter &painter) const
{
	updateTransforms ();
	painter.setTransform (_p_T_w, false);
}

/**
 * Returns the bounding rectangle for the visible rectangle, in plot coordinates
 *
 * Note that the y axis of QRectF points up and the y axis of the plot
 * coordinate system points down, so the top of the returned rectangle will
 * actually be the south of the bounding rectangle.
 */
QRectF PlotWidget::boundingRect_p () const
{
	updateTransforms ();
	return _w_T_p.mapRect (QRectF (rect ()));
}
