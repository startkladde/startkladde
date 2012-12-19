#include "src/flarm/flarmMap/NewFlarmMapWidget.h"

//#include <cassert>
#include <iostream>

#include <QModelIndex>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>

#include "src/util/qRect.h"
//#include "src/flarm/FlarmRecord.h"
//#include "src/numeric/Velocity.h"
//#include "src/numeric/GeoPosition.h"
#include "src/flarm/FlarmList.h"
//#include "src/util/qHash.h"
//#include "src/i18n/notr.h"
#include "src/nmea/GpsTracker.h"
//#include "src/util/qPointF.h"
//#include "src/util/qString.h"
#include "src/flarm/flarmMap/KmlReader.h"
//#include "src/qwt/SkPlotMagnifier.h"
//#include "src/qwt/SkPlotPanner.h"


// **************************
// ** StaticMarker methods **
// **************************

NewFlarmMapWidget::StaticMarker::StaticMarker (const GeoPosition &position, const QString &text, const QColor &backgroundColor):
	position (position), text (text), backgroundColor (backgroundColor)
{
}

NewFlarmMapWidget::StaticMarker::StaticMarker (const Kml::Marker &marker, const Kml::Style &style):
	position (marker.position), text (marker.name), backgroundColor (style.labelColor)
{
}

// *************************
// ** StaticCurve methods **
// *************************

NewFlarmMapWidget::StaticCurve::StaticCurve (const QVector<GeoPosition> &points, const QString &name, const QPen &pen):
	points (points), name (name), pen (pen)
{
}

NewFlarmMapWidget::StaticCurve::StaticCurve (const Kml::Path &path, const Kml::Style &style):
	points (path.positions.toVector ()), name (path.name), pen (style.linePen ())
{
}

NewFlarmMapWidget::StaticCurve::StaticCurve (const Kml::Polygon &polygon, const Kml::Style &style):
	points (polygon.positions.toVector ()), name (polygon.name), pen (style.linePen ())
{

}

// ******************
// ** Construction **
// ******************

NewFlarmMapWidget::NewFlarmMapWidget (QWidget *parent): QFrame (parent),
	_ownPositionColor (255,   0, 0, 127),
	_climbColor       (  0, 255, 0, 127),
	_descentColor     (255, 255, 0, 127),
	flarmList (NULL), gpsTracker (NULL),
	displayCenterPosition (0, 0), smallerRadius (2000),
	kmlStatus (kmlNone)
{
	_ownPositionText=tr ("Start"); // FIXME proper English word

//	// Setup the axes. Note that the aspect ratio may not be correct. This will
//	// be rectified by the first resize event we'll receive before the widget
//	// is shown.
//	double displayRadius=2000;
//	setAxisScale (QwtPlot::yLeft  , -displayRadius, displayRadius);
//	setAxisScale (QwtPlot::xBottom, -displayRadius, displayRadius);
//
//	// Setup the panner
//	QwtPlotPanner* panner = new SkPlotPanner (canvas ());
//	panner->setAxisEnabled (QwtPlot::yLeft, true);
//	panner->setAxisEnabled (QwtPlot::xBottom, true);
//	panner->setMouseButton (Qt::LeftButton);
//	panner->setEnabled (true);
//	connect (panner, SIGNAL (moved ()), this, SIGNAL (viewChanged ()));
//
//	// Setup the magnifier
//	QwtPlotMagnifier *magnifier = new SkPlotMagnifier (canvas ());
//	magnifier->setMouseButton (Qt::MidButton);
//	magnifier->setAxisEnabled (QwtPlot::yRight, true);
//	magnifier->setAxisEnabled (QwtPlot::xBottom, true);
//	// Positive value - mouse wheel down (back) means zooming out. This is the
//	// convention that many other applications, including Firefox and Gimp, use.
//	magnifier->setMouseFactor (1.05);
//	magnifier->setWheelFactor (1.1);
//	// Note that we cannot use QwtPlotMagnifier's keyboard zoom functionality
//	// because that also checks the modifier keys and these may be different for
//	// different keyboard layouts; for example, on the American layout the plus
//	// key requires the shift modifier. Also, this allows only on key each for
//	// zooming in and out. We therefore implement keyboard zooming ourselves.
//	// This requires the widget's focusPolicy to be set. As long as
//	// this->canvas()'s focusPolicy is not set, QwtPlotMagnifier's keyboard
//	// zooming won't get in the way. To be sure, we set the keyFactor to 1.
//	magnifier->setKeyFactor (1);
//	magnifier->setEnabled (true);
//	connect (magnifier, SIGNAL (rescaled ()), this, SIGNAL (viewChanged ()));
//
//	// Add the grid
//	QwtPlotGrid* grid = new QwtPlotGrid ();
//	grid->attach (this);
//
//	updateStaticData ();
//	refreshFlarmData ();
}

NewFlarmMapWidget::~NewFlarmMapWidget ()
{
}

// **************************
// ** Generic axis methods **
// **************************

///**
// * Gets the coordinates of the (primary) axes as QRectF
// *
// * @return
// */
//QRectF NewFlarmMapWidget::getAxesRect () const
//{
//	const QwtScaleDiv *xScaleDiv = axisScaleDiv (QwtPlot::xBottom);
//	const QwtScaleDiv *yScaleDiv = axisScaleDiv (QwtPlot::yLeft);
//
//	double left  =xScaleDiv->lowerBound ();
//	double right =xScaleDiv->upperBound ();
//	double bottom=yScaleDiv->lowerBound ();
//	double top   =yScaleDiv->upperBound ();
//
//	QPointF topLeft (left, top);
//	QPointF bottomRight (right, bottom);
//	QRectF axesRect (topLeft, bottomRight);
//
//	return axesRect;
//}
//
///**
// * Gets the current radius of the (primary) axes
// *
// * The axis radius is half the diameter of the axes range. For example, if the
// * range of an axis is from -1 to +5, the radius is +3. The radius is always
// * positive.
// *
// * @return a QPointF containing the radius of the (bottom) x and (left) y axis
// */
//QPointF NewFlarmMapWidget::getAxesRadius () const
//{
//	const QwtScaleDiv *xScaleDiv = axisScaleDiv (QwtPlot::xBottom);
//	const QwtScaleDiv *yScaleDiv = axisScaleDiv (QwtPlot::yLeft);
//
//	double xAxisRange = (xScaleDiv->upperBound () - xScaleDiv->lowerBound ())/2;
//	double yAxisRange = (yScaleDiv->upperBound () - yScaleDiv->lowerBound ())/2;
//
//	return QPointF (xAxisRange, yAxisRange);
//}
//
///**
// * Gets the current center of the (primary) axes
// *
// * The axes center is middle of the axes range. For example, if the range of an
// * axis is from -1 to +5, the center is +2. The center can be positive or
// * negative.
// *
// * @return a QPointF containing the center of the (bottom) x and (left) y axis
// */
//QPointF NewFlarmMapWidget::getAxesCenter () const
//{
//	const QwtScaleDiv *xScaleDiv = axisScaleDiv (QwtPlot::xBottom);
//	const QwtScaleDiv *yScaleDiv = axisScaleDiv (QwtPlot::yLeft);
//
//	double xAxisCenter = (xScaleDiv->upperBound () + xScaleDiv->lowerBound ())/2;
//	double yAxisCenter = (yScaleDiv->upperBound () + yScaleDiv->lowerBound ())/2;
//
//	return QPointF (xAxisCenter, yAxisCenter);
//}
//
///**
// * Sets the current axes, given the center and radius in both directions
// *
// * Note that this method does not emit the viewChanged signal because the graph
// * has not been replotted, so accessing its properties (e. g. the axes) may
// * result in an error.
// *
// * @param center the center for the (bottom) x and (left) y axis
// * @param radius the radius for the (bottom) x and (left) y axis
// * @see getAxesRadius
// * @see getAxesCenter
// */
//void NewFlarmMapWidget::setAxes (const QPointF &center, const QPointF &radius)
//{
//	setAxisScale (QwtPlot::xBottom, center.x () - radius.x (), center.x () + radius.x ());
//	setAxisScale (QwtPlot::yLeft  , center.y () - radius.y (), center.y () + radius.y ());
//}
//
///**
// * Sets the current axes, given the radius, while retaining the center
// *
// * @param radius the radius for the (bottom) x and (left) y axis
// * @see setAxes
// */
//void NewFlarmMapWidget::setAxesRadius (const QPointF &radius)
//{
//	setAxes (getAxesCenter (), radius);
//}
//
///**
// * Sets the current axes, given the center, while retaining the radius
// *
// * @param center the center for the (bottom) x and (left) y axis
// * @see setAxes
// */
//void NewFlarmMapWidget::setAxesCenter (const QPointF &center)
//{
//	setAxes (center, getAxesRadius ());
//}
//
///**
// * Sets the current axes, given the radius, while retaining the center
// *
// * This is a convenience method for setAxesRadius (const QPointF &radius).
// *
// * @param xRadius the radius for the (bottom) x axis
// * @param yRadius the radius for the (left) y axis
// */
//void NewFlarmMapWidget::setAxesRadius (double xRadius, double yRadius)
//{
//	setAxesRadius (QPointF (xRadius, yRadius));
//}
//
///**
// * A convenience method for setAxesRadius
// *
// * @param factor the zoom factor; a positive zoom factor sets a smaller radius,
// *               thereby enlarging the plot
// */
//void NewFlarmMapWidget::zoomAxes (double factor)
//{
//	setAxesRadius (getAxesRadius ()/factor);
//}
//
//void NewFlarmMapWidget::moveAxesCenter (const QPointF &offset)
//{
//	setAxesCenter (getAxesCenter ()+offset);
//}
//
//void NewFlarmMapWidget::moveAxesCenter (double xOffset, double yOffset)
//{
//	moveAxesCenter (QPointF (xOffset, yOffset));
//}
//
//
//// ****************
//// ** GUI events **
//// ****************

void NewFlarmMapWidget::keyPressEvent (QKeyEvent *event)
{
//	// Note that we don't use the magnifier for scaling; first, its rescale()
//	// method is protected and second, we'd have to store a pointer to it.
//
//	const double keyboardZoomFactor=1.1;
//
//	switch (event->key ())
//	{
//		case Qt::Key_Plus : case Qt::Key_BracketLeft : case Qt::Key_Equal:
//			zoomAxes (  keyboardZoomFactor); replot (); emit viewChanged (); break;
//		case Qt::Key_Minus: case Qt::Key_BracketRight:
//			zoomAxes (1/keyboardZoomFactor); replot (); emit viewChanged (); break;
//
//		case Qt::Key_Right: case Qt::Key_L: moveAxesCenter ( 0.1*min (getAxesRadius ()), 0); replot (); emit viewChanged (); break;
//		case Qt::Key_Left : case Qt::Key_H: moveAxesCenter (-0.1*min (getAxesRadius ()), 0); replot (); emit viewChanged (); break;
//		case Qt::Key_Up   : case Qt::Key_K: moveAxesCenter (0,  0.1*min (getAxesRadius ())); replot (); emit viewChanged (); break;
//		case Qt::Key_Down : case Qt::Key_J: moveAxesCenter (0, -0.1*min (getAxesRadius ())); replot (); emit viewChanged (); break;
//
//		default:
//			QwtPlot::keyPressEvent (event);
//			break;
//	}
}

void NewFlarmMapWidget::resizeEvent (QResizeEvent *event)
{
//	// Update the display radius such that the smaller value is retained and the
//	// aspect ratio matches the widget's aspect ratio.
//
//	// Don't update the radius if the widget size is zero in either direction
//	if (width () > 0 && height () > 0)
//	{
//		double smallerRadius=min (getAxesRadius ());
//		double widgetAspectRatio = width () / (double)height ();
//
//		if (widgetAspectRatio>=1)
//			// The widget is wider than high
//			setAxesRadius (smallerRadius*widgetAspectRatio, smallerRadius);
//		else
//			// The widget is higher than wide
//			setAxesRadius (smallerRadius, smallerRadius/widgetAspectRatio);
//	}
//
//	QwtPlot::resizeEvent (event);
//	replot ();
}

//// ****************
//// ** Flarm list **
//// ****************
//
///**
// * Sets the flarm list to use
// *
// * If a Flarm list was set before, it is replaced by the new Flarm list. If the
// * new Flarm list is the same as before, nothing is changed. Setting the Flarm
// * list to NULL (the default) effectively disables Flarm data display.
// *
// * This method calls replot().
// *
// * @param flarmList the new Flarm list. This view does not take ownership of the
// *                  Flarm list.
// */
void NewFlarmMapWidget::setFlarmList (FlarmList *flarmList)
{
	// FIXME do this in all classes using a model
	if (flarmList==this->flarmList)
		return;

	// Note that we can ignore layoutChanged because we don't consider the
	// order of the entries anyway, and we refer to entries by Flarm ID, not
	// index.

	if (this->flarmList)
	{
		disconnect (this->flarmList, SIGNAL (destroyed()), this, SLOT (flarmListDestroyed ()));
		disconnect (this->flarmList, SIGNAL (dataChanged (QModelIndex, QModelIndex)), this, SLOT (dataChanged (QModelIndex, QModelIndex)));
		disconnect (this->flarmList, SIGNAL (rowsInserted (QModelIndex, int, int)), this, SLOT (rowsInserted (QModelIndex, int, int)));
		disconnect (this->flarmList, SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),this, SLOT (rowsAboutToBeRemoved (QModelIndex, int, int)));
		disconnect (this->flarmList, SIGNAL (modelReset ()), this, SLOT (modelReset ()));
	}

	this->flarmList=flarmList;

	if (this->flarmList)
	{
		connect (this->flarmList, SIGNAL (destroyed()), this, SLOT (flarmListDestroyed ()));
		connect (this->flarmList, SIGNAL (dataChanged (QModelIndex, QModelIndex)), this, SLOT (dataChanged (QModelIndex, QModelIndex)));
		connect (this->flarmList, SIGNAL (rowsInserted (QModelIndex, int, int)), this, SLOT (rowsInserted (QModelIndex, int, int)));
		connect (this->flarmList, SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),this, SLOT (rowsAboutToBeRemoved (QModelIndex, int, int)));
		connect (this->flarmList, SIGNAL (modelReset ()), this, SLOT (modelReset ()));
	}

	// FIXME
//	refreshFlarmData ();
//	update ();
}

void NewFlarmMapWidget::setGpsTracker (GpsTracker *gpsTracker)
{
	if (this->gpsTracker)
	{
		disconnect (this->gpsTracker, SIGNAL (positionChanged (const GeoPosition &)), this, SLOT (ownPositionChanged (const GeoPosition &)));
	}

	this->gpsTracker=gpsTracker;

	if (this->gpsTracker)
	{
		connect (this->gpsTracker, SIGNAL (positionChanged (const GeoPosition &)), this, SLOT (ownPositionChanged (const GeoPosition &)));
	}

	if (this->gpsTracker)
	{
		ownPositionChanged (gpsTracker->getPosition ());
	}
}

// **********
// ** View **
// **********

/**
 * Sets the orientation such that the given compass direction is shown in the up
 * direction in the window
 *
 * The default is north up, or upDirection=0. upDirection=90° means east up.
 */
void NewFlarmMapWidget::setOrientation (const Angle &upDirection)
{
	transform=QTransform ();
	transform.rotateRadians (upDirection.toRadians ());

	// Schedule a repaint
	update ();
}

/**
 * Updates the static curves for the new position
 *
 * Call this method when the own position changes.
 */
void NewFlarmMapWidget::ownPositionChanged (const GeoPosition &ownPosition)
{
	// Only redraw if either the current own position is invalid (i. e., we
	// didn't draw yet), the new own position is invalid (GPS reception lost),
	// or the position changed by more than a given threshold.
	// The typical noise on the Flarm GPS data seems to be substantially less
	// than 1 m. Currently, the threshold is set to 0, so the widget is redrawn
	// every time a position is received (typically once per second).
	if (
		!_ownPosition.isValid () ||
		! ownPosition.isValid () ||
		ownPosition.distanceTo (_ownPosition)>0)
	{
		_ownPosition=ownPosition;
		update ();
		emit ownPositionUpdated (); // FIXME what for?
	}
}

bool NewFlarmMapWidget::isOwnPositionKnown () const
{
	return _ownPosition.isValid ();
}


//// *****************
//// ** Static data **
//// *****************

/**
 * Adds, updates or removes a static marker at the origin (i. e. the own
 * position)
 *
 * @param text the text to display at the own position. If empty, the own
 * position is not shown.
 * @param color the background color of the label
 */
void NewFlarmMapWidget::setOwnPositionLabel (const QString &text, const QColor &color)
{
	_ownPositionText=text;
	_ownPositionColor=color;
}

///**
// * Updates the curve data structures for all static curves
// *
// * This must be called whenever the geometry of the curves as drawn on screen
// * changes, for example when the own position or the tranform changes.
// *
// * This method does not call replot(). You have to call it yourself to update
// * the display.
// */
//void NewFlarmMapWidget::updateStaticData ()
//{
//	bool valid=ownPosition.isValid ();
//
//	allStaticPoints.clear ();
//
//	// Markers
//	foreach (const StaticMarker &marker, staticMarkers)
//	{
//		if (valid)
//		{
//			QPointF point (marker.position.relativePositionTo (ownPosition));
//			QPointF transformedPoint=transform.map (point);
//			marker.marker->setValue (transformedPoint);
//			allStaticPoints.append (transformedPoint);
//		}
//		marker.marker->setVisible (valid);
//	}
//
//	// Curves
//	foreach (const StaticCurve &curve, staticCurves)
//	{
//		if (valid)
//		{
//			QPolygonF polygon (GeoPosition::relativePositionTo (curve.points, ownPosition));
//			QPolygonF transformedPolygon=transform.map (polygon);
//			curve.data->setSamples (transformedPolygon);
//			allStaticPoints.append (transformedPolygon.toList ());
//		}
//		curve.curve->setVisible (valid);
//	}
//
//}
//
//
//// ***********************************
//// ** Flarm data individual updates **
//// ***********************************
//
///**
// * Sets the marker for a Flarm record to the "minimal" form
// *
// * This method does not call replot(). You have to call it yourself to update
// * the display.
// *
// * @param marker the marker to modify
// * @param record the Flarm record the marker is associated with
// */
//void NewFlarmMapWidget::updateMarkerMinimal (QwtPlotMarker *marker, const FlarmRecord &record)
//{
//	marker->setVisible (true);
//
//	Q_UNUSED (record);
//
//	// Symbol: small blue cross
//	QwtSymbol *symbol=new QwtSymbol (); // Will be deleted by the marker
//	symbol->setStyle (QwtSymbol::Cross);
//	symbol->setSize (8);
//	symbol->setPen (QPen (Qt::blue));
//	marker->setSymbol (symbol);
//
//	// Label: none
//	marker->setLabel (QwtText ());
//}
//
///**
// * Sets the marker for a Flarm record to the "verbose" form
// *
// * This method does not call replot(). You have to call it yourself to update
// * the display.
// *
// * @param marker the marker to modify
// * @param record the Flarm record the marker is associated with
// */
//void NewFlarmMapWidget::updateMarkerVerbose (QwtPlotMarker *marker, const FlarmRecord &record)
//{
//	marker->setVisible (true);
//
//	// Symbol: none
//	marker->setSymbol (NULL);
//
//	// Label: verbose text
//	QwtText text (qnotr ("%1\n%2/%3/%4")
//		.arg (record.getRegistration ())
//		.arg (record.getRelativeAltitude ())
//		.arg (record.getGroundSpeed () / Velocity::km_h)
//		.arg (record.getClimbRate (), 0, 'f', 1));
//
//	if (record.getClimbRate () > 0.0)
//		text.setBackgroundBrush (QBrush (climbColor));
//	else
//		text.setBackgroundBrush (QBrush (descentColor));
//
//	marker->setLabel (text);
//}
//
///**
// * Updates the trail curve for a Flarm
// *
// * This method does not call replot(). You have to call it yourself to update
// * the display.
// *
// * @param curve the crve to modify
// * @param record the Flarm record the curve is associated with
// */
//void NewFlarmMapWidget::updateTrail (QwtPlotCurve *curve, const FlarmRecord &record)
//{
//	curve->setVisible (true);
//
//	// Prepare data. The data will be deleted by the curve.
//	QPolygonF polygon (record.getPreviousRelativePositions ().toVector ());
//	// Will be deleted by the curve
//	QwtPointSeriesData *data = new QwtPointSeriesData (transform.map (polygon));
//	curve->setData (data);
//}
//
//
//// ****************
//// ** Flarm data **
//// ****************
//
///**
// * Adds the plot data for a given Flarm record
// *
// * This must be called exactly once for each Flarm record after it is added.
// * updateFlarmData may only be called after this method has been called for the
// * given Flarm record.
// *
// * This method also calls updateFlarmData, so after the call to addFlarmData,
// * the data will be up to date.
// *
// * This method does not call replot(). You have to call it yourself to update
// * the display.
// *
// * @param record the new Flarm record
// */
//void NewFlarmMapWidget::addFlarmData (const FlarmRecord &record)
//{
//	// Create, attach and store the marker
//	QwtPlotMarker *marker = new QwtPlotMarker ();
//	marker->attach (this);
//	flarmMarkers.insert (record.getFlarmId (), marker);
//
//	// Create, attach and store the curve
//	QwtPlotCurve* curve = new QwtPlotCurve ("history");
//	curve->attach (this);
//	flarmCurves.insert (record.getFlarmId (), curve);
//
//	// Setup the curve
//	QPen pen;
//	pen.setWidth (2);
//	curve->setPen (pen);
//	curve->setRenderHint (QwtPlotItem::RenderAntialiased);
//
//	// Update the data (marker and trail)
//	updateFlarmData (record);
//}
//
///**
// * Updates the plot data (marker and curve) for a given Flarm record
// *
// * This must be called whenever a Flarm record is updated.
// *
// * This method does not call replot(). You have to call it yourself to update
// * the display.
// *
// * @param record the updated Flarm record
// */
//void NewFlarmMapWidget::updateFlarmData (const FlarmRecord &record)
//{
//	QwtPlotMarker *marker=flarmMarkers.value (record.getFlarmId (), NULL);
//	QwtPlotCurve  *curve =flarmCurves .value (record.getFlarmId (), NULL);
//
//	// Always set the position, even if the marker is not visible
//	marker->setValue (transform.map (record.getRelativePosition ()));
//
//	switch (record.getState ())
//	{
//		case FlarmRecord::stateStarting:
//		case FlarmRecord::stateFlying:
//		case FlarmRecord::stateLanding:
//			// Verbose marker, trail
//			updateMarkerVerbose (marker, record);
//			updateTrail (curve, record);
//			break;
//		case FlarmRecord::stateOnGround:
//			// Minimal marker, no trail
//			updateMarkerMinimal (marker, record);
//			curve->setVisible (false);
//			break;
//		case FlarmRecord::stateUnknown:
//		case FlarmRecord::stateFlyingFar:
//			// No marker, no trail
//			marker->setVisible (false);
//			curve->setVisible (false);
//			break;
//		// no default
//	}
//}
//
///**
// * Removes the plot data for a given Flarm record
// *
// * This must be called exactly once for each Flarm record before (!) it is
// * removed. updateFlarmData may not be called after this method has been called
// * for the given Flarm record.
// *
// * This method does not call replot(). You have to call it yourself to update
// * the display.
// *
// * @param record the new Flarm record
// */
//void NewFlarmMapWidget::removeFlarmData (const FlarmRecord &record)
//{
//	// Items will be detached automatically on deletion.
//	QString flarmId=record.getFlarmId ();
//	removeAndDeleteIfExists (flarmMarkers, flarmId);
//	removeAndDeleteIfExists (flarmCurves , flarmId);
//}
//
///**
// * Refreshes the plot data for all Flarm records in the Flarm list
// *
// * This is done by first removing all plot data and then adding the plot data
// * by calling addFlarmData for each Flarm record in the Flarm list.
// *
// * This method can be called even if there is no Flarm list. All plot data will
// * still be removed and nothing will be added.
// *
// * This method does not call replot(). You have to call it yourself to update
// * the display.
// */
void NewFlarmMapWidget::refreshFlarmData ()
{
//	// Items will be detached automatically on deletion.
//	clearAndDelete (flarmMarkers);
//	clearAndDelete (flarmCurves);
//
//	// Only draw if we have a Flarm list
//	if (flarmList)
//	{
//		for (int i=0, n=flarmList->size (); i<n; ++i)
//		{
//			const FlarmRecord &record=flarmList->at (i);
//			addFlarmData (record);
//		}
//	}
}


//// **********************
//// ** Flarm list slots **
//// **********************
//
///**
// * Called after one or more rows have been inserted into the Flarm lsit. Adds
// * the Flarm data for the new row(s).
// */
void NewFlarmMapWidget::rowsInserted (const QModelIndex &parent, int start, int end)
{
//	Q_UNUSED (parent);
//
//	if (flarmList)
//		for (int i=start; i<=end; ++i)
//			addFlarmData (flarmList->at (i));
//
//	replot ();
}

///**
// * Called after one or more rows have changed in the Flarm list. Updates the
// * Flarm data for the changed row(s).
// */
void NewFlarmMapWidget::dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
//	if (flarmList)
//		for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
//			updateFlarmData (flarmList->at (i));
//
//	// Disable this to replot only when the own position changes
//	replot ();
}

///**
// * Called before (!) one or more rows are removed from the Flarm list. Removes
// * the Flarm data for the row(s) to be removed.
// */
void NewFlarmMapWidget::rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
{
//	Q_UNUSED (parent);
//
//	if (flarmList)
//		for (int i=start; i<=end; ++i)
//			removeFlarmData (flarmList->at (i));
//
//	replot ();
}

///**
// * Called after the Flarm list has changed completely. Refreshes all Flarm data.
// */
void NewFlarmMapWidget::modelReset ()
{
//	refreshFlarmData ();
//	replot ();
}

///**
// * Called before the Flarm list is destroyed
// *
// * This method sets the Flarm list to NULL, meaning "no Flarm list", to prevent
// * further accesses to the model.
// */
void NewFlarmMapWidget::flarmListDestroyed ()
{
//	this->flarmList=NULL;
//	modelReset ();
}


// *********
// ** KML **
// *********

/**
 * Note that if multiple KML files have been loaded (by multiple calls to
 * readKml), this only reflects the status of the last file.
 */
NewFlarmMapWidget::KmlStatus NewFlarmMapWidget::getKmlStatus () const
{
	return kmlStatus;
}

/**
 * Never call this method except from readKml
 *
 * This method only exists to facilitate setting kmlStatus in readKml and to
 * allow the compiler to issue a warning in case no status is assigned.
 */
NewFlarmMapWidget::KmlStatus NewFlarmMapWidget::readKmlImplementation (const QString &filename)
{
	QString effectiveFilename=filename.trimmed ();
	if (effectiveFilename.isEmpty ())
		return kmlNone;

	KmlReader kmlReader;
	KmlReader::ReadResult readResult=kmlReader.read (effectiveFilename);

	switch (readResult)
	{
		case KmlReader::readNotFound:   return kmlNotFound  ;
		case KmlReader::readOpenError:  return kmlReadError ;
		case KmlReader::readParseError: return kmlParseError;
		case KmlReader::readOk:         break; // Go on, process it
		// no default
	}

	if (kmlReader.isEmpty ())
		return kmlEmpty;

	// For each KML marker in the KML file, add a static marker
	foreach (const Kml::Marker &marker, kmlReader.markers)
	{
		Kml::Style style=kmlReader.findStyle (marker.styleUrl);
		staticMarkers.append (StaticMarker (marker, style));
		std::cout << "add static marker" << std::endl;
	}

	// For each KML path in the KML file, add a static curve
	foreach (const Kml::Path &path, kmlReader.paths)
	{
		Kml::Style style=kmlReader.findStyle (path.styleUrl);
		staticCurves.append (StaticCurve (path, style));
	}

	// For each KML polygon in the KML file, add a static curve
	foreach (const Kml::Polygon &polygon, kmlReader.polygons)
	{
		Kml::Style style=kmlReader.findStyle (polygon.styleUrl);
		staticCurves.append (StaticCurve (polygon, style));
	}

	// Something changed, so we have to schedule a repaint
	update ();

	return kmlOk;
}

NewFlarmMapWidget::KmlStatus NewFlarmMapWidget::readKml (const QString &filename)
{
	kmlStatus=readKmlImplementation (filename);
	return kmlStatus;
}


//// **********
//// ** View **
//// **********
//
bool NewFlarmMapWidget::isOwnPositionVisible () const
{
//	return getAxesRect ().contains (0, 0);
}

bool NewFlarmMapWidget::isAnyStaticElementVisible () const
{
//	if (!ownPosition.isValid ())
//		return false;
//
//	QRectF axesRect=getAxesRect ();
//
//	foreach (const QPointF &point, allStaticPoints)
//		if (axesRect.contains (point))
//			return true;
//
//	return false;
}

bool NewFlarmMapWidget::findClosestStaticElement (double *distance, Angle *bearing) const
{
//	QPointF viewCenter=getAxesCenter ();
//
//	QPointF closestPoint;
//	double closestDistanceSquared=-1;
//
//	foreach (const QPointF &p, allStaticPoints)
//	{
//		QPointF point=p-viewCenter;
//
//		double distanceSquared=lengthSquared (point);
//
//		if (closestDistanceSquared<0 || distanceSquared<closestDistanceSquared)
//		{
//			closestPoint=point;
//			closestDistanceSquared=distanceSquared;
//		}
//	}
//
//	if (closestDistanceSquared>=0)
//	{
//		if (distance) (*distance)=sqrt (closestDistanceSquared);
//		// Note the transposition - atan2 calculates mathematical angle
//		// (starting at x, going counter-clockwise), we need geographical angle
//		// (starting at y, going clockwise).
//		if (bearing)  (*bearing)=Angle::atan2 (transposed (closestPoint));
//
//		return true;
//	}
//	else
//	{
//		return false;
//	}
}

void NewFlarmMapWidget::resetPosition ()
{
//	setAxesCenter (QPointF (0, 0));
//	replot ();
//	emit viewChanged ();
}


// **************
// ** Painting **
// **************

QPoint NewFlarmMapWidget::mapFromLocal (const QPointF &localPoint)
{
	// Transform the position to the display coordinate system (x/y in meters,
	// centered at the display center, orientation up)
	QPointF displayPoint=localPoint-displayCenterPosition;
	displayPoint=transform.map (displayPoint);

	// Transform the position to the plot coordinate system (x/y in pixels,
	// centered at the widget center, orientation up)
	// FIXME This assumes that the vertical radius is the smaller one
	double verticalRadius=smallerRadius;
	double horizontalRadius=smallerRadius*width()/height();

	QPoint plotPoint=QPoint (displayPoint.x ()/horizontalRadius*width ()/2, displayPoint.y ()/verticalRadius*height ()/2);

	// Transform the position to the centered painter coordinate system, which
	// has y down
	QPoint painterPoint (plotPoint.x (), -plotPoint.y ());

	return painterPoint;
}

QPoint NewFlarmMapWidget::mapFromGeographic (const GeoPosition &geoPosition)
{
	// The position is specified in the geographic coordinate system (latitude/
	// longitude, centered at lat=0/lon=0, north up)

	// Transform the position to the local coordinate system (eastness/northness
	// in meters, centered at the own position, north up)
	QPointF localPoint (geoPosition.relativePositionTo (_ownPosition));

	return mapFromLocal (localPoint);
}

QPolygon NewFlarmMapWidget::mapFromLocal (const QPolygonF &localPolygon)
{
	QPolygon result;

	foreach (const QPointF &localPoint, localPolygon)
		result.append (mapFromLocal (localPoint));

	return result;
}

QPolygon NewFlarmMapWidget::mapFromGeographic (const QVector<GeoPosition> &geoPositions)
{
	QPolygon result;

	foreach (const GeoPosition &geoPosition, geoPositions)
		result.append (mapFromGeographic (geoPosition));

	return result;
}

void drawCenteredText (QPainter &painter, const QPoint &position, const QString &text)
{
	QSize size=painter.fontMetrics ().size (0, text);
	QRect rect=centeredQRect (position, size);
	painter.fillRect (rect, painter.brush ());
	painter.drawText (rect, text);
}

void NewFlarmMapWidget::paintEvent (QPaintEvent *event)
{
	(void)event;
	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing, true);

	// The plot coordinate system is centered in the middle of the
	painter.translate (width ()/2, height ()/2);

	painter.setPen (Qt::black);

	// Draw the own position
	painter.setBrush (_ownPositionColor);
	drawCenteredText (painter, QPoint (0, 0), _ownPositionText);

	// We can only draw the static data if the own position is known, because it
	// is specified in absolute (earth) coordinates and the display coordinate
	// system is centered at the own position.
	if (_ownPosition.isValid ())
	{
		// Draw all static markers
		foreach (const StaticMarker &marker, staticMarkers)
		{
			QPoint p=mapFromGeographic (marker.position);
			painter.setBrush (marker.backgroundColor);
			drawCenteredText (painter, p, marker.text);
		}

		// Draw all static paths
		foreach (const StaticCurve &curve, staticCurves)
		{
			QPolygon p=mapFromGeographic (curve.points);
			painter.setPen (curve.pen);
			painter.drawPolyline (p);
		}

	}

}
