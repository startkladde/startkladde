#include <src/flarm/FlarmMapWidget.h>

#include <cassert>
#include <iostream>

#include <QModelIndex>
#include <QResizeEvent>
#include <QKeyEvent>

#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_canvas.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>

#include "src/flarm/FlarmRecord.h"
#include "src/numeric/Velocity.h"
#include "src/numeric/GeoPosition.h"
#include "src/flarm/FlarmList.h"
#include "src/util/qHash.h"
#include "src/i18n/notr.h"
#include "src/nmea/GpsTracker.h"
#include "src/util/qPointF.h"
#include "src/util/qString.h"
#include "src/flarm/KmlReader.h"

// ******************
// ** Construction **
// ******************

FlarmMapWidget::FlarmMapWidget (QWidget *parent): QwtPlot (parent),
	climbColor   (  0, 255, 0, 127),
	descentColor (255, 255, 0, 127),
	flarmList (NULL), gpsTracker (NULL),
	ownPositionMarker (NULL)
{
	// Setup the axes. Note that the aspect ratio may not be correct. This will
	// be rectified by the first resize event we'll receive before the widget
	// is shown.
	double displayRadius=2000;
	setAxisScale (QwtPlot::yLeft  , -displayRadius, displayRadius);
	setAxisScale (QwtPlot::xBottom, -displayRadius, displayRadius);

	// Setup the panner
	QwtPlotPanner* panner = new QwtPlotPanner (canvas ());
	panner->setAxisEnabled (QwtPlot::yLeft, true);
	panner->setAxisEnabled (QwtPlot::xBottom, true);
	panner->setMouseButton (Qt::LeftButton);
	panner->setEnabled (true);

	// Setup the magnifier
	QwtPlotMagnifier *magnifier = new QwtPlotMagnifier (canvas ());
	magnifier->setMouseButton (Qt::MidButton);
	magnifier->setAxisEnabled (QwtPlot::yRight, true);
	magnifier->setAxisEnabled (QwtPlot::xBottom, true);
	// Positive value - mouse wheel down (back) means zooming out. This is the
	// convention that many other applications, including Firefox and Gimp, use.
	magnifier->setMouseFactor (1.05);
	magnifier->setWheelFactor (1.1);
	// Note that we cannot use QwtPlotMagnifier's keyboard zoom functionality
	// because that also checks the modifier keys and these may be different for
	// different keyboard layouts; for example, on the American layout the plus
	// key requires the shift modifier. Also, this allows only on key each for
	// zooming in and out. We therefore implement keyboard zooming ourselves.
	// This requires the widget's focusPolicy to be set. As long as
	// this->canvas()'s focusPolicy is not set, QwtPlotMagnifier's keyboard
	// zooming won't get in the way. To be sure, we set the keyFactor to 1.
	magnifier->setKeyFactor (1);
	magnifier->setEnabled (true);

	// Add the grid
	QwtPlotGrid* grid = new QwtPlotGrid ();
	grid->attach (this);

	// Add static markers
	setOwnPositionLabel ("Start", QColor (255, 0, 0, 127));

	updateStaticData ();
	refreshFlarmData ();
	replot ();
}

FlarmMapWidget::~FlarmMapWidget ()
{
}

// **************************
// ** Generic axis methods **
// **************************

/**
 * Gets the current radius of the (primary) axes
 *
 * The axis radius is half the diameter of the axes range. For example, if the
 * range of an axis is from -1 to +5, the radius is +3. The radius is always
 * positive.
 *
 * @return a QPointF containing the radius of the (bottom) x and (left) y axis
 */
QPointF FlarmMapWidget::getAxesRadius () const
{
	const QwtScaleDiv *xScaleDiv = axisScaleDiv (QwtPlot::xBottom);
	const QwtScaleDiv *yScaleDiv = axisScaleDiv (QwtPlot::yLeft);

	double xAxisRange = (xScaleDiv->upperBound () - xScaleDiv->lowerBound ())/2;
	double yAxisRange = (yScaleDiv->upperBound () - yScaleDiv->lowerBound ())/2;

	return QPointF (xAxisRange, yAxisRange);
}

/**
 * Gets the current center of the (primary) axes
 *
 * The axes center is middle of the axes range. For example, if the range of an
 * axis is from -1 to +5, the center is +2. The center can be positive or
 * negative.
 *
 * @return a QPointF containing the center of the (bottom) x and (left) y axis
 */
QPointF FlarmMapWidget::getAxesCenter () const
{
	const QwtScaleDiv *xScaleDiv = axisScaleDiv (QwtPlot::xBottom);
	const QwtScaleDiv *yScaleDiv = axisScaleDiv (QwtPlot::yLeft);

	double xAxisCenter = (xScaleDiv->upperBound () + xScaleDiv->lowerBound ())/2;
	double yAxisCenter = (yScaleDiv->upperBound () + yScaleDiv->lowerBound ())/2;

	return QPointF (xAxisCenter, yAxisCenter);
}

/**
 * Sets the current axes, given the center and radius in both directions
 *
 * @param center the center for the (bottom) x and (left) y axis
 * @param radius the radius for the (bottom) x and (left) y axis
 * @see getAxesRadius
 * @see getAxesCenter
 */
void FlarmMapWidget::setAxes (const QPointF &center, const QPointF &radius)
{
	setAxisScale (QwtPlot::xBottom, center.x () - radius.x (), center.x () + radius.x ());
	setAxisScale (QwtPlot::yLeft  , center.y () - radius.y (), center.y () + radius.y ());
}

/**
 * Sets the current axes, given the radius, while retaining the center
 *
 * @param radius the radius for the (bottom) x and (left) y axis
 * @see setAxes
 */
void FlarmMapWidget::setAxesRadius (const QPointF &radius)
{
	setAxes (getAxesCenter (), radius);
}

/**
 * Sets the current axes, given the center, while retaining the radius
 *
 * @param center the center for the (bottom) x and (left) y axis
 * @see setAxes
 */
void FlarmMapWidget::setAxesCenter (const QPointF &center)
{
	setAxes (center, getAxesRadius ());
}

/**
 * Sets the current axes, given the radius, while retaining the center
 *
 * This is a convenience method for setAxesRadius (const QPointF &radius).
 *
 * @param xRadius the radius for the (bottom) x axis
 * @param yRadius the radius for the (left) y axis
 */
void FlarmMapWidget::setAxesRadius (double xRadius, double yRadius)
{
	setAxesRadius (QPointF (xRadius, yRadius));
}

/**
 * A convenience method for setAxesRadius
 *
 * @param factor the zoom factor; a positive zoom factor sets a smaller radius,
 *               thereby enlarging the plot
 */
void FlarmMapWidget::zoomAxes (double factor)
{
	setAxesRadius (getAxesRadius ()/factor);
}

void FlarmMapWidget::moveAxesCenter (const QPointF &offset)
{
	setAxesCenter (getAxesCenter ()+offset);
}

void FlarmMapWidget::moveAxesCenter (double xOffset, double yOffset)
{
	moveAxesCenter (QPointF (xOffset, yOffset));
}


// ****************
// ** GUI events **
// ****************

void FlarmMapWidget::keyPressEvent (QKeyEvent *event)
{
	// Note that we don't use the magnifier for scaling; first, its rescale()
	// method is protected and second, we'd have to store a pointer to it.

	const double keyboardZoomFactor=1.1;

	switch (event->key ())
	{
		case Qt::Key_Plus : case Qt::Key_BracketLeft : case Qt::Key_Equal:
			zoomAxes (  keyboardZoomFactor); replot (); break;
		case Qt::Key_Minus: case Qt::Key_BracketRight:
			zoomAxes (1/keyboardZoomFactor); replot (); break;

		case Qt::Key_Right: case Qt::Key_L: moveAxesCenter ( 0.1*min (getAxesRadius ()), 0); replot (); break;
		case Qt::Key_Left : case Qt::Key_H: moveAxesCenter (-0.1*min (getAxesRadius ()), 0); replot (); break;
		case Qt::Key_Up   : case Qt::Key_K: moveAxesCenter (0,  0.1*min (getAxesRadius ())); replot (); break;
		case Qt::Key_Down : case Qt::Key_J: moveAxesCenter (0, -0.1*min (getAxesRadius ())); replot (); break;

		default:
			QwtPlot::keyPressEvent (event);
			break;
	}
}

void FlarmMapWidget::resizeEvent (QResizeEvent *event)
{
	// Update the display radius such that the smaller value is retained and the
	// aspect ratio matches the widget's aspect ratio.

	// Don't update the radius if the widget size is zero in either direction
	if (width () > 0 && height () > 0)
	{
		double smallerRadius=min (getAxesRadius ());
		double widgetAspectRatio = width () / (double)height ();

		if (widgetAspectRatio>=1)
			// The widget is wider than high
			setAxesRadius (smallerRadius*widgetAspectRatio, smallerRadius);
		else
			// The widget is higher than wide
			setAxesRadius (smallerRadius, smallerRadius/widgetAspectRatio);
	}

	QwtPlot::resizeEvent (event);
	replot ();
}


// ****************
// ** Flarm list **
// ****************

/**
 * Sets the flarm list to use
 *
 * If a Flarm list was set before, it is replaced by the new Flarm list. If the
 * new Flarm list is the same as before, nothing is changed. Setting the Flarm
 * list to NULL (the default) effectively disables Flarm data display.
 *
 * This method calls replot().
 *
 * @param flarmList the new Flarm list. This view does not take ownership of the
 *                  Flarm list.
 */
void FlarmMapWidget::setFlarmList (FlarmList *flarmList)
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

	refreshFlarmData ();
	replot ();
}

void FlarmMapWidget::setGpsTracker (GpsTracker *gpsTracker)
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
}

// **********
// ** View **
// **********

/**
 * Sets the orientation such that the given compass direction is shown in the up
 * direction in the window
 *
 * The default is north up, or upDirection=0. upDirection=90° means east up.
 *
 * This method calls replot().
 */
void FlarmMapWidget::setOrientation (const Angle &upDirection)
{
	transform=QTransform ();
	transform.rotateRadians (upDirection.toRadians ());

	updateStaticData ();
	// FIXME we just want to update them here, not refresh them
	refreshFlarmData ();
	replot ();
}

/**
 * Updates the static curves for the new position
 *
 * Call this method when the own position changes.
 *
 * This method calls replot().
 */
void FlarmMapWidget::ownPositionChanged (const GeoPosition &ownPosition)
{
	// Only redraw if either the reference position for the static data is
	// invalid (i. e., we didn't draw yet), or the position changed by more
	// than a given threshold. The usual noise on the Flarm GPS data seems to be
	// substantially less than 1 m. Currently, the threshold is set to 0, so the
	// static data is redrawn every time a position is received (typically once
	// per second).
	if (
		!this->ownPosition.isValid () ||
		ownPosition.distanceTo (this->ownPosition)>0)
	{
		this->ownPosition=ownPosition;
		updateStaticData ();
		replot ();
	}
}



// *****************
// ** Static data **
// *****************

/**
 * Adds a static curve to the list of static curves
 *
 * This method does not call updateStaticData() or replot(). You have to call it
 * yourself to update the display.
 *
 * The created marker is set to invisible initially. It will be set to visible
 * as soon as the static data is updated when an own position is receive.
 *
 * @param name a name for the curve, e. g. "airfield"
 * @param points a vector of points that make up the curve. The curve will not
 *               be closed automatically.
 * @param pen the pen to use for drawing the curve
 */
void FlarmMapWidget::addStaticCurve (const QString &name, const QVector<GeoPosition> &points, QPen pen)
{
	StaticCurve curve;

	curve.name=name;

	curve.points=points;

	curve.data=new QwtPointSeriesData (); // Deleted by curve (FIXME really?)

	curve.curve=new QwtPlotCurve (name); // Deleted by QwtPlot
	curve.curve->setRenderHint (QwtPlotItem::RenderAntialiased);
	curve.curve->setPen (pen);
	curve.curve->setData (curve.data);
	curve.curve->setVisible (false);
	curve.curve->attach (this);

	staticCurves.append (curve);
}

/**
 * Adds, updates or removes a static marker at the origin (i. e. the own
 * position)
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 *
 * @param text the text to dsplay on the label. If empty, the label is removed.
 * @param color the background color of the label
 */
void FlarmMapWidget::setOwnPositionLabel (const QString &text, const QColor &color)
{
	if (text.isEmpty ())
	{
		// The test is empty. Remove the label from the plot.
		// Delete it because if we only removed it, it would be hard to tell
		// whether we'd have to delete it in the destructor. If the marker has
		// not yet been created, this is a noop.
		delete ownPositionMarker;
		ownPositionMarker=NULL;
	}
	else
	{
		// If the marker does not exist yet, create and attach it
		if (!ownPositionMarker)
		{
			ownPositionMarker=new QwtPlotMarker (); // Deleted by plot or manually
			ownPositionMarker->setValue (QPointF (0, 0));
			ownPositionMarker->attach (this);
		}

		// Set the marker's text and color
		QwtText qwtText (text);
		qwtText.setBackgroundBrush (QBrush (color));
		ownPositionMarker->setLabel (qwtText);
	}
}

/**
 * Adds a static marker at a given position
 *
 * This method does not call updateStaticData() or replot(). You have to call it
 * yourself to update the display.
 *
 * The created marker is set to invisible initially. It will be set to visible
 * as soon as the static data is updated when an own position is receive.
 *
 * @param text the text to display on the label
 * @param color the background color of the label
 * @param point the position, in meters east and north of the own position
 */
void FlarmMapWidget::addStaticMarker (const QString &text, const GeoPosition &position, const QColor &color)
{
	StaticMarker marker;

	marker.position=position;

	QwtText qwtText (text);
	qwtText.setBackgroundBrush (QBrush (color));

	marker.marker=new QwtPlotMarker (); // Deleted by QwtPlot
//	marker.marker->setRenderHint (QwtPlotItem::RenderAntialiased); // FIXME want?
	marker.marker->setLabel (qwtText);
	marker.marker->setVisible (false);
	marker.marker->attach (this);

	staticMarkers.append (marker);
}

/**
 * Updates the curve data structures for all static curves
 *
 * This must be called whenever the geometry of the curves as drawn on screen
 * changes, for example when the own position or the tranform changes.
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 */
void FlarmMapWidget::updateStaticData ()
{
	bool valid=ownPosition.isValid ();

	// Curves
	foreach (const StaticCurve &curve, staticCurves)
	{
		if (valid)
		{
			QPolygonF polygon (GeoPosition::relativePositionTo (curve.points, ownPosition));
			curve.data->setSamples (transform.map (polygon));
		}
		curve.curve->setVisible (valid);
	}

	// Markers
	foreach (const StaticMarker &marker, staticMarkers)
	{
		if (valid)
		{
			QPointF point (marker.position.relativePositionTo (ownPosition));
			marker.marker->setValue (transform.map (point));
		}
		marker.marker->setVisible (valid);
	}

}


// ***********************************
// ** Flarm data individual updates **
// ***********************************

/**
 * Sets the marker for a Flarm record to the "minimal" form
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 *
 * @param marker the marker to modify
 * @param record the Flarm record the marker is associated with
 */
void FlarmMapWidget::updateMarkerMinimal (QwtPlotMarker *marker, const FlarmRecord &record)
{
	marker->setVisible (true);

	Q_UNUSED (record);

	// Symbol: small blue cross
	QwtSymbol *symbol=new QwtSymbol (); // Will be deleted by the marker
	symbol->setStyle (QwtSymbol::Cross);
	symbol->setSize (8);
	symbol->setPen (QPen (Qt::blue));
	marker->setSymbol (symbol);

	// Label: none
	marker->setLabel (QwtText ());
}

/**
 * Sets the marker for a Flarm record to the "verbose" form
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 *
 * @param marker the marker to modify
 * @param record the Flarm record the marker is associated with
 */
void FlarmMapWidget::updateMarkerVerbose (QwtPlotMarker *marker, const FlarmRecord &record)
{
	marker->setVisible (true);

	// Symbol: none
	marker->setSymbol (NULL);

	// Label: verbose text
	QwtText text (qnotr ("%1\n%2/%3/%4")
		.arg (record.getRegistration ())
		.arg (record.getRelativeAltitude ())
		.arg (record.getGroundSpeed () / Velocity::km_h)
		.arg (record.getClimbRate (), 0, 'f', 1));

	if (record.getClimbRate () > 0.0)
		text.setBackgroundBrush (QBrush (climbColor));
	else
		text.setBackgroundBrush (QBrush (descentColor));

	marker->setLabel (text);
}

/**
 * Updates the trail curve for a Flarm
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 *
 * @param curve the crve to modify
 * @param record the Flarm record the curve is associated with
 */
void FlarmMapWidget::updateTrail (QwtPlotCurve *curve, const FlarmRecord &record)
{
	curve->setVisible (true);

	// Prepare data. The data will be deleted by the curve.
	QPolygonF polygon (record.getPreviousRelativePositions ().toVector ());
	// Will be deleted by the curve
	QwtPointSeriesData *data = new QwtPointSeriesData (transform.map (polygon));
	curve->setData (data);
}


// ****************
// ** Flarm data **
// ****************

/**
 * Adds the plot data for a given Flarm record
 *
 * This must be called exactly once for each Flarm record after it is added.
 * updateFlarmData may only be called after this method has been called for the
 * given Flarm record.
 *
 * This method also calls updateFlarmData, so after the call to addFlarmData,
 * the data will be up to date.
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 *
 * @param record the new Flarm record
 */
void FlarmMapWidget::addFlarmData (const FlarmRecord &record)
{
	// Create, attach and store the marker
	QwtPlotMarker *marker = new QwtPlotMarker ();
	marker->attach (this);
	flarmMarkers.insert (record.getFlarmId (), marker);

	// Create, attach and store the curve
	QwtPlotCurve* curve = new QwtPlotCurve ("history");
	curve->attach (this);
	flarmCurves.insert (record.getFlarmId (), curve);

	// Setup the curve
	QPen pen;
	pen.setWidth (2);
	curve->setPen (pen);
	curve->setRenderHint (QwtPlotItem::RenderAntialiased);

	// Update the data (marker and trail)
	updateFlarmData (record);
}

/**
 * Updates the plot data (marker and curve) for a given Flarm record
 *
 * This must be called whenever a Flarm record is updated.
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 *
 * @param record the updated Flarm record
 */
void FlarmMapWidget::updateFlarmData (const FlarmRecord &record)
{
	QwtPlotMarker *marker=flarmMarkers.value (record.getFlarmId (), NULL);
	QwtPlotCurve  *curve =flarmCurves .value (record.getFlarmId (), NULL);

	// Always set the position, even if the marker is not visible
	marker->setValue (transform.map (record.getRelativePosition ()));

	switch (record.getState ())
	{
		case FlarmRecord::stateStarting:
		case FlarmRecord::stateFlying:
		case FlarmRecord::stateLanding:
			// Verbose marker, trail
			updateMarkerVerbose (marker, record);
			updateTrail (curve, record);
			break;
		case FlarmRecord::stateOnGround:
			// Minimal marker, no trail
			updateMarkerMinimal (marker, record);
			curve->setVisible (false);
			break;
		case FlarmRecord::stateUnknown:
		case FlarmRecord::stateFlyingFar:
			// No marker, no trail
			marker->setVisible (false);
			curve->setVisible (false);
			break;
		// no default
	}
}

/**
 * Removes the plot data for a given Flarm record
 *
 * This must be called exactly once for each Flarm record before (!) it is
 * removed. updateFlarmData may not be called after this method has been called
 * for the given Flarm record.
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 *
 * @param record the new Flarm record
 */
void FlarmMapWidget::removeFlarmData (const FlarmRecord &record)
{
	// Items will be detached automatically on deletion.
	QString flarmId=record.getFlarmId ();
	removeAndDeleteIfExists (flarmMarkers, flarmId);
	removeAndDeleteIfExists (flarmCurves , flarmId);
}

/**
 * Refreshes the plot data for all Flarm records in the Flarm list
 *
 * This is done by first removing all plot data and then adding the plot data
 * by calling addFlarmData for each Flarm record in the Flarm list.
 *
 * This method can be called even if there is no Flarm list. All plot data will
 * still be removed and nothing will be added.
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 */
void FlarmMapWidget::refreshFlarmData ()
{
	// Items will be detached automatically on deletion.
	clearAndDelete (flarmMarkers);
	clearAndDelete (flarmCurves);

	// Only draw if we have a Flarm list
	if (flarmList)
	{
		for (int i=0, n=flarmList->size (); i<n; ++i)
		{
			const FlarmRecord &record=flarmList->at (i);
			addFlarmData (record);
		}
	}
}


// **********************
// ** Flarm list slots **
// **********************

/**
 * Called after one or more rows have been inserted into the Flarm lsit. Adds
 * the Flarm data for the new row(s).
 */
void FlarmMapWidget::rowsInserted (const QModelIndex &parent, int start, int end)
{
	Q_UNUSED (parent);

	if (flarmList)
		for (int i=start; i<=end; ++i)
			addFlarmData (flarmList->at (i));

	replot ();
}

/**
 * Called after one or more rows have changed in the Flarm list. Updates the
 * Flarm data for the changed row(s).
 */
void FlarmMapWidget::dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	if (flarmList)
		for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
			updateFlarmData (flarmList->at (i));

	// Disable this to replot only when the own position changes
	replot ();
}

/**
 * Called before (!) one or more rows are removed from the Flarm list. Removes
 * the Flarm data for the row(s) to be removed.
 */
void FlarmMapWidget::rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
{
	Q_UNUSED (parent);

	if (flarmList)
		for (int i=start; i<=end; ++i)
			removeFlarmData (flarmList->at (i));

	replot ();
}

/**
 * Called after the Flarm list has changed completely. Refreshes all Flarm data.
 */
void FlarmMapWidget::modelReset ()
{
	refreshFlarmData ();
	replot ();
}

/**
 * Called before the Flarm list is destroyed
 *
 * This method sets the Flarm list to NULL, meaning "no Flarm list", to prevent
 * further accesses to the model.
 */
void FlarmMapWidget::flarmListDestroyed ()
{
	this->flarmList=NULL;
	modelReset ();
}


// *********
// ** KML **
// *********

void FlarmMapWidget::readKml (const QString &filename)
{
	KmlReader kmlReader;
	kmlReader.read (filename);

	// FIXME error indication: file not found

	 // FIXME clean up
	 // FIXME honor color, line thickness, other attributes?

	foreach (const KmlReader::Marker &marker, kmlReader.markers)
		addStaticMarker (marker.name, marker.position, marker.color);

	foreach (const KmlReader::Path &path, kmlReader.paths)
		addStaticCurve (path.name, path.positions.toVector (), QPen (path.color));

	foreach (const KmlReader::Polygon &polygon, kmlReader.polygons)
		addStaticCurve (polygon.name, polygon.positions.toVector (), QPen (polygon.color));

	replot ();
}
