#include "src/flarm/flarmMap/NewFlarmMapWidget.h"

//#include <cassert>
#include <iostream>

#include <QModelIndex>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>

// FIXME: must emit viewChanged

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
	localCenter (0, 0), smallerRadius (2000),
	kmlStatus (kmlNone)
{
	_ownPositionText=tr ("Start"); // FIXME proper English word

	// FIXME panning
	// FIXME mouse drag magnification
}

NewFlarmMapWidget::~NewFlarmMapWidget ()
{
}

// ****************
// ** GUI events **
// ****************

void NewFlarmMapWidget::zoom (double factor)
{
	smallerRadius/=factor;
	update ();
	emit viewChanged ();
}

void NewFlarmMapWidget::scroll (double x, double y)
{
	// Scrolling takes place in plot/view coordinates
	QPointF viewCenter=transform.map (localCenter);
	viewCenter+=QPointF (x, y);
	localCenter=transform.transposed ().map (viewCenter);
	update ();
	emit viewChanged ();
}

void NewFlarmMapWidget::keyPressEvent (QKeyEvent *event)
{
	const double keyboardDoubleCount=8;
	const double keyboardZoomFactor=pow (2, 1/keyboardDoubleCount);

	switch (event->key ())
	{
		// Zoom in
		case Qt::Key_Plus:        zoom (keyboardZoomFactor); break;
		case Qt::Key_BracketLeft: zoom (keyboardZoomFactor); break;
		case Qt::Key_Equal:       zoom (keyboardZoomFactor); break;

		// Zoom out
		case Qt::Key_Minus:        zoom (1/keyboardZoomFactor); break;
		case Qt::Key_BracketRight: zoom (1/keyboardZoomFactor); break;

		// Scroll
		case Qt::Key_Right: case Qt::Key_L: scroll ( 0.1*smallerRadius, 0); break;
		case Qt::Key_Left : case Qt::Key_H: scroll (-0.1*smallerRadius, 0); break;
		case Qt::Key_Up   : case Qt::Key_K: scroll (0,  0.1*smallerRadius); break;
		case Qt::Key_Down : case Qt::Key_J: scroll (0, -0.1*smallerRadius); break;

		// Other
		default: QFrame::keyPressEvent (event); break;
	}
	// FIXME always call super, and use accept()/ignore()
}

void NewFlarmMapWidget::wheelEvent (QWheelEvent *event)
{
	// FIXME use zoom(), and zoom around the mouse wheel position
	// Mouse wheel down (back) means zooming out. This is the convention that
	// many other applications, including Firefox and Gimp, use.
	double degrees=event->delta ()/(double)8;
	smallerRadius*=pow (2, -degrees/120);
	update ();
}


// ****************
// ** Flarm list **
// ****************

/**
 * Sets the Flarm list to use
 *
 * If a Flarm list was set before, it is replaced by the new Flarm list. If the
 * new Flarm list is the same as before, nothing is changed. Setting the Flarm
 * list to NULL (the default) effectively disables Flarm data display.
 *
 * This view does not take ownership of the Flarm list.
 */
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

	update ();
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


// ****************
// ** Transforms **
// ****************

// Coordinate systems:
//   * geographic: latitude/longitude, origin at the equator/zero meridian
//   * local: east/north in meters, origin at the own position
//   * view: right/up in meters, orientation up, origin at the own position
//   * plot: right/up in pixels, orientation up, origin at the display center
//   * widget: right/down in pixels, orientation up, origin in the upper left
//

// FIXME transform matrix, and reverse
QPoint NewFlarmMapWidget::transformLocalToWidget (const QPointF &localPoint)
{
	// Local to view
	QPointF viewPoint =transform.map (localPoint);
	QPointF viewCenter=transform.map (localCenter);

	// View to plot
	QPointF viewRelativeToCenter=viewPoint-viewCenter;
	QPoint plotPoint (
		viewRelativeToCenter.x () * width  () / (getXRadius ()*2),
		viewRelativeToCenter.y () * height () / (getYRadius ()*2));

	// Plot to widget
	QPoint widgetPoint (
		width  ()/2 + plotPoint.x (),
		height ()/2 - plotPoint.y ());

	return widgetPoint;
}

QPointF NewFlarmMapWidget::transformWidgetToLocal (const QPoint &widgetPoint)
{
	qDebug () << "widget" << widgetPoint;
	QPointF plotPoint (widgetPoint.x ()-width ()/2, height()/2-widgetPoint.y ());
	qDebug () << "plot" << plotPoint;
	QPointF viewRelativeToCenter (plotPoint.x ()/width()*getXRadius()*2, plotPoint.y()/height ()*getYRadius()*2);
	QPointF viewCenter=transform.map (localCenter);
	QPointF viewPoint=viewRelativeToCenter+viewCenter;
	qDebug () << "view" << viewPoint;
	QPointF localPoint=transform.transposed().map (viewPoint);
	qDebug () << "local" << localPoint;
	return localPoint;
}

/**
 * Undefined if the own position is invalid
 */
QPoint NewFlarmMapWidget::transformGeographicToWidget (const GeoPosition &geoPosition)
{
	// Geographic to local
	QPointF localPoint (geoPosition.relativePositionTo (_ownPosition));

	// Local to widget
	return transformLocalToWidget (localPoint);
}


// Vectorized transform
QPolygon NewFlarmMapWidget::transformLocalToWidget (const QPolygonF &localPolygon)
{
	QPolygon result;

	foreach (const QPointF &localPoint, localPolygon)
		result.append (transformLocalToWidget (localPoint));

	return result;
}

// Vectorized transform
QPolygon NewFlarmMapWidget::transformGeographicToWidget (const QVector<GeoPosition> &geoPositions)
{
	QPolygon result;

	foreach (const GeoPosition &geoPosition, geoPositions)
		result.append (transformGeographicToWidget (geoPosition));

	return result;
}


// **************
// ** Painting **
// **************

void drawCenteredText (QPainter &painter, const QPoint &position, const QString &text)
{
	QSize size=painter.fontMetrics ().size (0, text);
	QRect rect=centeredQRect (position, size);
	painter.fillRect (rect, painter.brush ());
	painter.drawText (rect, text);
}

double NewFlarmMapWidget::getLargerRadius () const
{
	double widgetAspectRatio = width () / (double)height ();

	if (widgetAspectRatio>=1)
		// The widget is wider than high
		return smallerRadius*widgetAspectRatio;
	else
		// The widget is higher than wide
		return smallerRadius/widgetAspectRatio;

}

double NewFlarmMapWidget::getXRadius () const
{
	double widgetAspectRatio = width () / (double)height ();

	if (widgetAspectRatio>=1)
		// The widget is wider than high
		return smallerRadius*widgetAspectRatio;
	else
		// The widget is higher than wide
		return smallerRadius;

}

double NewFlarmMapWidget::getYRadius () const
{
	double widgetAspectRatio = width () / (double)height ();

	if (widgetAspectRatio>=1)
		// The widget is wider than high
		return smallerRadius;
	else
		// The widget is higher than wide
		return smallerRadius/widgetAspectRatio;
}

void NewFlarmMapWidget::paintCoordinateSystem (QPainter &painter)
{
	painter.save ();
	QPen pen=painter.pen ();
	pen.setWidthF (0.5);
	painter.setPen (pen);

	double radiusIncrement=1000;

	double largerRadius=getLargerRadius ();

	// FIXME minimum pixel distance
	// FIXME draw all visible, even when the origin is scrolled out of view
	// FIXME draw distances
	for (double radius=radiusIncrement; radius<=largerRadius; radius+=radiusIncrement)
	{
		// FIXME inefficient
		QSize size (radius*width ()/getXRadius (), radius*height ()/getYRadius ());
		QRect rect=centeredQRect (transformLocalToWidget (QPoint (0, 0)), size);
		painter.drawArc (rect, 0, 16*360);
	}

	// FIXME why do we have to transpose?
	// FIXME arrow and "N"
	// FIXME move when scrolling
	painter.setTransform (transform.transposed (), true);
	painter.drawLine (0, 0, 0, -qMin (width (), height ())*1/4);

	painter.restore ();
}

void NewFlarmMapWidget::paintEvent (QPaintEvent *event)
{
	(void)event;
	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing, true);

	painter.setPen (Qt::black);

//	paintCoordinateSystem (painter);

	// Draw the own position
	painter.setBrush (_ownPositionColor);
	drawCenteredText (painter, transformLocalToWidget (QPoint (0, 0)), _ownPositionText);

	// We can only draw the static data if the own position is known, because it
	// is specified in absolute (earth) coordinates and the display coordinate
	// system is centered at the own position.
	if (_ownPosition.isValid ())
	{
		// Draw all static paths
		foreach (const StaticCurve &curve, staticCurves)
		{
			QPolygon p=transformGeographicToWidget (curve.points);
			painter.setPen (curve.pen);
			painter.drawPolyline (p);
		}

		// Draw all static markers
		foreach (const StaticMarker &marker, staticMarkers)
		{
			QPoint p=transformGeographicToWidget (marker.position);
			painter.setBrush (marker.backgroundColor);
			drawCenteredText (painter, p, marker.text);
		}
	}
}

void NewFlarmMapWidget::mousePressEvent (QMouseEvent *event)
{
	localCenter=transformWidgetToLocal (event->pos ());
	update ();
//	if (event->buttons ()==Qt::LeftButton)
//	{
//		drag
//	}
}

void NewFlarmMapWidget::mouseReleaseEvent (QMouseEvent *event)
{

}

void NewFlarmMapWidget::mouseMoveEvent (QMouseEvent *event)
{

}

