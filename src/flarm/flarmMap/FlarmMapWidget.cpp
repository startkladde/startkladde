#include "src/flarm/flarmMap/FlarmMapWidget.h"

#include <iostream>

#include <QBrush>
#include <QKeyEvent>
#include <QModelIndex>
#include <QPainter>
#include <QPen>
#include <QResizeEvent>

#include "src/flarm/FlarmList.h"
#include "src/flarm/FlarmRecord.h"
#include "src/flarm/flarmMap/KmlReader.h"
#include "src/i18n/notr.h"
#include "src/nmea/GpsTracker.h"
#include "src/numeric/Velocity.h"
#include "src/util/qPainter.h"
#include "src/util/qPointF.h"
#include "src/util/qRect.h"
#include "src/util/qSize.h"

// Implementation note: you must call updateView whenever the view (i. e., the
// visible region extents) changes. This will schedule a repaint and cause the
// transforms to be recalculated, which will, in turn, emit the viewChanged
// signal.

// **************************
// ** StaticMarker methods **
// **************************

FlarmMapWidget::StaticMarker::StaticMarker (const GeoPosition &position, const QString &text, const QColor &backgroundColor):
	position (position), text (text), backgroundColor (backgroundColor)
{
}

FlarmMapWidget::StaticMarker::StaticMarker (const Kml::Marker &marker, const Kml::Style &style):
	position (marker.position), text (marker.name), backgroundColor (style.labelColor)
{
}

// *************************
// ** StaticCurve methods **
// *************************

FlarmMapWidget::StaticCurve::StaticCurve (const QVector<GeoPosition> &points, const QString &name, const QPen &pen):
	points (points), name (name), pen (pen)
{
}

FlarmMapWidget::StaticCurve::StaticCurve (const Kml::Path &path, const Kml::Style &style):
	points (path.positions.toVector ()), name (path.name), pen (style.linePen ())
{
}

FlarmMapWidget::StaticCurve::StaticCurve (const Kml::Polygon &polygon, const Kml::Style &style):
	points (polygon.positions.toVector ()), name (polygon.name), pen (style.linePen ())
{

}

// *******************
// ** Image methods **
// *******************

FlarmMapWidget::Image::Image (const Kml::GroundOverlay &groundOverlay)
{
	// TODO error handling - .load returns false
	pixmap.load (groundOverlay.filename);

	northEast=GeoPosition (groundOverlay.north, groundOverlay.east);
	southWest=GeoPosition (groundOverlay.south, groundOverlay.west);
	northWest=GeoPosition (groundOverlay.north, groundOverlay.west);
	southEast=GeoPosition (groundOverlay.south, groundOverlay.east);
}


// ******************
// ** Construction **
// ******************

FlarmMapWidget::FlarmMapWidget (QWidget *parent): QFrame (parent),
	_ownPositionColor (255,   0, 0, 127),
	_climbColor       (  0, 255, 0, 127),
	_descentColor     (255, 255, 0, 127),
	flarmList (NULL), gpsTracker (NULL),
	kmlStatus (kmlNone),
	_center_local (0, 0), _radius (2000), _orientation (Angle::fromDegrees (0)),
	transformsValid (false),
	scrollDragging (false), zoomDragging (false), rotateDragging (false),
	_keyboardZoomDoubleCount (8), _mouseDragZoomDoubleDistance (50),
	_mouseWheelZoomDoubleAngle (Angle::fromDegrees (120)),
	_mouseDragRotationDistance (-360)
{
	_ownPositionText=tr ("Start"); // FIXME proper English word
}

FlarmMapWidget::~FlarmMapWidget ()
{
}


// ****************
// ** Transforms **
// ****************

void FlarmMapWidget::invalidateTransforms ()
{
	transformsValid=false;
}

void FlarmMapWidget::updateTransforms () const
{
	if (transformsValid)
		return;
	transformsValid=true;

	// The view coordinate system is rotated clockwise by the orientation with
	// respect to the local coordinate system. For example, for an orientation
	// of 45° (northeast up), the view coordinate system is rotated clockwise by
	// 45°. QTransform::rotateRadians rotates counter-clockwise.
	viewSystem_local=QTransform ();
	viewSystem_local.rotateRadians (-_orientation.toRadians ());
	localSystem_view=viewSystem_local.inverted ();

	// The plot coordinate system has its origin at the center of the display,
	// is parallel to the view coordinate system and uses pixel units instead of
	// meter units.
	QPointF center_view=_center_local*localSystem_view;
	plotSystem_view=QTransform ();
	plotSystem_view.translate (center_view.x (), center_view.y ());
	plotSystem_view.scale (2*getXRadius ()/width (), 2*getYRadius ()/height ());
	viewSystem_plot=plotSystem_view.inverted ();

	// The widget coordinate system has its origin at the top left corner of the
	// display and the y axis down instead of up. Otherwise, it is parallel to
	// the plot coordinate system and has the same size.
	widgetSystem_plot=QTransform ();
	widgetSystem_plot.translate (-width ()/2, height ()/2);
	widgetSystem_plot.scale (1, -1);
	plotSystem_widget=widgetSystem_plot.inverted ();

	// The transform from the local to the widget coordinate system is given by
	// the product of the individual transformations.
	widgetSystem_local=widgetSystem_plot*plotSystem_view*viewSystem_local;
	localSystem_widget=widgetSystem_local.inverted ();

	emit viewChanged ();
}








// ****************
// ** GUI events **
// ****************

void FlarmMapWidget::keyPressEvent (QKeyEvent *event)
{
	// Note that we don't call event->accept (). The documentation for QWidget::
	// keyPressEvent recommends that implementations do not call the superclass
	// method if they act upon the key.
	const double keyboardZoomFactor=pow (2, 1/_keyboardZoomDoubleCount);

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
		case Qt::Key_Right: case Qt::Key_L: scroll ( 0.1*_radius, 0); break;
		case Qt::Key_Left : case Qt::Key_H: scroll (-0.1*_radius, 0); break;
		case Qt::Key_Up   : case Qt::Key_K: scroll (0,  0.1*_radius); break;
		case Qt::Key_Down : case Qt::Key_J: scroll (0, -0.1*_radius); break;

		// Other
		default: QFrame::keyPressEvent (event); break;
	}
}

void FlarmMapWidget::wheelEvent (QWheelEvent *event)
{
	// FIXME zoom around the mouse wheel position
	// Mouse wheel down (back) means zooming out. This is the convention that
	// many other applications, including Firefox and Gimp, use.
	Angle angle=Angle::fromDegrees (event->delta ()/(double)8);
	zoom (pow (2, -angle/_mouseWheelZoomDoubleAngle));
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

	update ();
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
void FlarmMapWidget::setOrientation (const Angle &orientation)
{
	Angle o=orientation.normalized ();

	bool changed=(o!=_orientation);
	_orientation=o;

	if (changed)
		emit orientationChanged ();

	// Schedule a repaint
	updateView ();
}

// FIXME document
Angle FlarmMapWidget::orientation ()
{
	return _orientation;
}

/**
 * Updates the static curves for the new position
 *
 * Call this method when the own position changes.
 */
void FlarmMapWidget::ownPositionChanged (const GeoPosition &ownPosition)
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
		updateView ();
		emit ownPositionUpdated ();
	}
}

bool FlarmMapWidget::isOwnPositionKnown () const
{
	return _ownPosition.isValid ();
}


// *****************
// ** Static data **
// *****************

/**
 * Adds, updates or removes a static marker at the origin (i. e. the own
 * position)
 *
 * @param text the text to display at the own position. If empty, the own
 * position is not shown.
 * @param color the background color of the label
 */
void FlarmMapWidget::setOwnPositionLabel (const QString &text, const QColor &color)
{
	_ownPositionText=text;
	_ownPositionColor=color;
	update ();
}


// ****************
// ** Flarm data **
// ****************

/**
 * Adds a marker for a given Flarm record
 *
 * This must be called exactly once for each Flarm record after it is added. If
 * the plane data changes subsequently, updatePlaneMarker must be called. You
 * may only call updatePlaneMarker after addPlaneMarker has been called with
 * the respective Flarm record.
 *
 * This method also calls updatePlaneMarker.
 */
void FlarmMapWidget::addPlaneMarker (const FlarmRecord &record)
{
	planeMarkers.insert (record.getFlarmId (), PlaneMarker ());
	updatePlaneMarker (record);
}

/**
 * Updates the plane marker data for a given Flarm record
 *
 * This method must be called whenever a Flarm record is updated. It also
 * schedules a repaint of the widget.
 */
void FlarmMapWidget::updatePlaneMarker (const FlarmRecord &record)
{
	PlaneMarker &marker=planeMarkers[record.getFlarmId ()];

	// Always set the position, even if the marker is not visible
	marker.position_local=record.getRelativePosition ();
	marker.trail_local=record.getPreviousRelativePositions ().toVector ();

	switch (record.getState ())
	{
		case FlarmRecord::stateStarting:
		case FlarmRecord::stateFlying:
		case FlarmRecord::stateLanding:
			marker.style=PlaneMarker::verbose;

			marker.text=qnotr ("%1\n%2/%3/%4")
				.arg (record.getRegistration ())
				.arg (record.getRelativeAltitude ())
				.arg (record.getGroundSpeed () / Velocity::km_h)
				.arg (record.getClimbRate (), 0, 'f', 1);

			if (record.getClimbRate () > 0.0)
				marker.color=_climbColor;
			else
				marker.color=_descentColor;

			break;
		case FlarmRecord::stateOnGround:
			marker.style=PlaneMarker::minimal;
			break;
		case FlarmRecord::stateUnknown:
		case FlarmRecord::stateFlyingFar:
			marker.style=PlaneMarker::invisible;
			break;
		// no default
	}

	// Schedule a repaint
	update ();
}

/**
 * Removes the plane marker for a given Flarm record
 *
 * This must be called exactly once for each Flarm record before (!) it is
 * removed. updateFlarmData may not be called after this method has been called
 * for the given Flarm record. This method also schedules a repaint of the
 * widget.
 */
void FlarmMapWidget::removePlaneMarker (const FlarmRecord &record)
{
	planeMarkers.remove (record.getFlarmId ());
	update ();
}

/**
 * Refreshes the plane markers for all Flarm records in the Flarm list
 *
 * This is done by first removing all plane markers and then adding the plane
 * markers by calling addPlaneMarker for each Flarm record in the Flarm list.
 *
 * This method can be called even if there is no Flarm list. All plot data will
 * still be removed and nothing will be added.
 */
void FlarmMapWidget::refreshPlaneMarkers ()
{
	planeMarkers.clear ();

	// Only draw if we have a Flarm list
	if (flarmList)
	{
		for (int i=0, n=flarmList->size (); i<n; ++i)
		{
			const FlarmRecord &record=flarmList->at (i);
			addPlaneMarker (record);
		}
	}
}


// **********************
// ** Flarm list slots **
// **********************

/**
 * Called after one or more rows have been inserted into the Flarm list. Adds
 * the Flarm data for the new row(s).
 */
void FlarmMapWidget::rowsInserted (const QModelIndex &parent, int start, int end)
{
	Q_UNUSED (parent);

	if (flarmList)
		for (int i=start; i<=end; ++i)
			addPlaneMarker (flarmList->at (i));
}

/**
 * Called after one or more rows have changed in the Flarm list. Updates the
 * Flarm data for the changed row(s).
 */
void FlarmMapWidget::dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	if (flarmList)
		for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
			updatePlaneMarker (flarmList->at (i));
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
			removePlaneMarker (flarmList->at (i));
}

/**
 * Called after the Flarm list has changed completely. Refreshes all Flarm data.
 */
void FlarmMapWidget::modelReset ()
{
	refreshPlaneMarkers ();
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

/**
 * Note that if multiple KML files have been loaded (by multiple calls to
 * readKml), this only reflects the status of the last file.
 */
FlarmMapWidget::KmlStatus FlarmMapWidget::getKmlStatus () const
{
	return kmlStatus;
}

/**
 * Never call this method except from readKml
 *
 * This method only exists to facilitate setting kmlStatus in readKml and to
 * allow the compiler to issue a warning in case no status is assigned.
 */
FlarmMapWidget::KmlStatus FlarmMapWidget::readKmlImplementation (const QString &filename)
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
	foreach (const Kml::Marker &kmlMarker, kmlReader.markers)
	{
		Kml::Style style=kmlReader.findStyle (kmlMarker.styleUrl);
		StaticMarker staticMarker (kmlMarker, style);
		staticMarkers.append (staticMarker);
		allStaticPositions.append (kmlMarker.position);
	}

	// For each KML path in the KML file, add a static curve
	foreach (const Kml::Path &path, kmlReader.paths)
	{
		Kml::Style style=kmlReader.findStyle (path.styleUrl);
		StaticCurve staticCurve=StaticCurve (path, style);
		staticCurves.append (staticCurve);
		allStaticPositions.append (staticCurve.points.toList ());
	}

	// For each KML polygon in the KML file, add a static curve
	foreach (const Kml::Polygon &polygon, kmlReader.polygons)
	{
		Kml::Style style=kmlReader.findStyle (polygon.styleUrl);
		StaticCurve staticCurve=StaticCurve (polygon, style);
		staticCurves.append (staticCurve);
		allStaticPositions.append (staticCurve.points.toList ());
	}

	// For each ground overlay in the KML file, add an image
	foreach (const Kml::GroundOverlay &overlay, kmlReader.groundOverlays)
	{
		Image image=Image (overlay);
		images.append (image);
		allStaticPositions.append (image.northWest);
		allStaticPositions.append (image.northEast);
		allStaticPositions.append (image.southWest);
		allStaticPositions.append (image.southEast);
	}

	// Something changed, so we have to schedule a repaint
	update ();

	return kmlOk;
}

FlarmMapWidget::KmlStatus FlarmMapWidget::readKml (const QString &filename)
{
	kmlStatus=readKmlImplementation (filename);
	return kmlStatus;
}


// **********
// ** View **
// **********

bool FlarmMapWidget::isOwnPositionVisible () const
{
	updateTransforms ();

	QRectF visibleRect_widget=rect ();

	QPointF ownPosition_local (0, 0);
	QPointF ownPosition_widget=ownPosition_local*localSystem_widget;

	return visibleRect_widget.contains (ownPosition_widget.toPoint ());
}

bool FlarmMapWidget::isAnyStaticElementVisible () const
{
	if (!_ownPosition.isValid ())
		return false;

	QRectF visibleRect_widget=rect ();

	foreach (const GeoPosition &position, allStaticPositions)
	{
		QPointF position_widget=transformGeographicToWidget (position);
		if (visibleRect_widget.contains (position_widget))
			return true;
	}

	return false;
}

bool FlarmMapWidget::findClosestStaticElement (double *distance, Angle *bearing) const
{
	updateTransforms ();

	QPointF closestPoint_local;
	double closestDistanceSquared=-1;

	foreach (const GeoPosition &p, allStaticPositions)
	{
		// In the local coordinate system
		QPointF point_local=p.relativePositionTo (_ownPosition);

		// Relative to the display center
		QPointF relativePoint_local=point_local-_center_local;

		double distanceSquared=lengthSquared (relativePoint_local);

		if (closestDistanceSquared<0 || distanceSquared<closestDistanceSquared)
		{
			closestPoint_local=relativePoint_local;
			closestDistanceSquared=distanceSquared;
		}
	}

	if (closestDistanceSquared>=0)
	{
		if (distance) (*distance)=sqrt (closestDistanceSquared);
		// Note the transposition - atan2 calculates mathematical angle
		// (starting at x, going counter-clockwise), we need geographical angle
		// (starting at y, going clockwise).
		if (bearing)  (*bearing)=Angle::atan2 (transposed (closestPoint_local));

		return true;
	}
	else
	{
		return false;
	}
}

void FlarmMapWidget::resetPosition ()
{
	_center_local=QPointF (0, 0);
	updateView ();
}


double FlarmMapWidget::getXRadius () const
{
	double widgetAspectRatio = width () / (double)height ();

	if (widgetAspectRatio>=1)
		// The widget is wider than high
		return _radius*widgetAspectRatio;
	else
		// The widget is higher than wide
		return _radius;

}

double FlarmMapWidget::getYRadius () const
{
	double widgetAspectRatio = width () / (double)height ();

	if (widgetAspectRatio>=1)
		// The widget is wider than high
		return _radius;
	else
		// The widget is higher than wide
		return _radius/widgetAspectRatio;
}

void FlarmMapWidget::mousePressEvent (QMouseEvent *event)
{
	updateTransforms ();

	if (event->button ()==Qt::LeftButton)
	{
		QPointF dragLocation_widget=event->posF ();
		dragLocation_local=dragLocation_widget*widgetSystem_local;
		scrollDragging=true;
	}
	else if (event->button ()==Qt::RightButton)
	{
		zoomDragStartPosition_widget=event->pos ();
		zoomDragStartRadius=_radius;
		zoomDragging=true;

		rotateDragStartPosition_widget=event->pos ();
		rotateDragStartOrientation=_orientation;
		qDebug () <<"setted rotateDragStartOrientation to " << rotateDragStartOrientation;
		rotateDragging=true;
	}
}

void FlarmMapWidget::mouseReleaseEvent (QMouseEvent *event)
{
	if (event->button ()==Qt::LeftButton)
		scrollDragging=false;
	else if (event->button ()==Qt::RightButton)
	{
		zoomDragging=false;
		rotateDragging=false;
	}
}

void FlarmMapWidget::mouseMoveEvent (QMouseEvent *event)
{
	updateTransforms ();

	QPointF mousePosition_widget=event->posF ();
	QPointF mousePosition_local=mousePosition_widget*widgetSystem_local;

	emit mouseMoved (mousePosition_local);

	if (scrollDragging)
	{
		// We want the dragged location (dragLocation) to be at the mouse position
		// (mousePosition). We therefore calculate the location that is currently
		// at the mouse position, determine the difference and correct the center
		// location. This has to be done in local coordinates because this is the
		// coordinate system the center location is stored in.

		// Calculate the location that is currently displayed at the mouse position,
		// in local coordinates.
		QPointF currentLocation_widget=mousePosition_widget;
		QPointF currentLocation_local=currentLocation_widget*widgetSystem_local;

		// The location that is supposed to be displayed at the mouse position, in
		// local coordinates, is given by dragLocation_local.

		_center_local += dragLocation_local-currentLocation_local;
		updateView ();
	}

	if (zoomDragging)
	{
		// FIXME zoom around the initial mouse position
		int deltaY=event->pos ().y () - zoomDragStartPosition_widget.y ();
		// FIXME add a zoomTo method?
		_radius=zoomDragStartRadius*pow (2, deltaY/_mouseDragZoomDoubleDistance);
		updateView ();
	}

	if (rotateDragging)
	{
		int deltaX=event->pos ().x () - rotateDragStartPosition_widget.x ();
		Angle deltaAngle=Angle::fullCircle ()*deltaX/_mouseDragRotationDistance;
		setOrientation (rotateDragStartOrientation+deltaAngle);
	}
}



// **********
// ** View **
// **********

void FlarmMapWidget::zoom (double factor)
{
	_radius/=factor;

	updateView ();
}

void FlarmMapWidget::scroll (double dx, double dy) // In meters
{
	updateTransforms ();

	QTransform t;
	t.rotateRadians (_orientation.toRadians ());
	// Scrolling takes place in plot/view coordinates
	QPointF center_view=_center_local*localSystem_view;
	center_view+=QPointF (dx, dy);
	_center_local=center_view*viewSystem_local;

	updateView ();
}

// Coordinate systems:
//   * geographic: latitude/longitude, origin at the equator/zero meridian
//   * local: east/north in meters, origin at the own position
//   * view: right/up in meters, orientation up, origin at the own position
//   * plot: right/up in pixels, orientation up, origin at the display center
//   * widget: right/down in pixels, orientation up, origin in the upper left

void FlarmMapWidget::updateView ()
{
	invalidateTransforms ();
	update ();
}

/**
 * Undefined if the own position is invalid
 */
QPointF FlarmMapWidget::transformGeographicToWidget (const GeoPosition &geoPosition) const
{
	updateTransforms ();

	// Geographic to local
	QPointF point_local (geoPosition.relativePositionTo (_ownPosition));

	// Local to widget
	return point_local*localSystem_widget;
}

// Vectorized transform
QPolygonF FlarmMapWidget::transformGeographicToWidget (const QVector<GeoPosition> &geoPositions) const
{
	QPolygonF result;

	// FIXME we can transform a QPolygon directly
	foreach (const GeoPosition &geoPosition, geoPositions)
		result.append (transformGeographicToWidget (geoPosition));

	return result;
}


void FlarmMapWidget::resizeEvent (QResizeEvent *event)
{
	(void)event;
	updateView ();
}

void FlarmMapWidget::leaveEvent (QEvent *event)
{
	(void)event;
	emit mouseLeft ();
}



// **************
// ** Painting **
// **************

// Notes for the paint* methods:
//   * The painter passed as an argument is set to widget coordinates. It may be
//     freely modified
//   * The updateTransforms method does not have to be called; this is done in
//     paintEvent ().
//   * All prerequisites (like the validity of the own position) have to be
//     checked by the methods. If the prerequisites are not met, the method
//     should return right away.

void FlarmMapWidget::paintImages (QPainter &painter)
{
	if (!_ownPosition.isValid ())
		return;

	painter.save ();

	// The y axis of the pixmap points down, so we use a coordinate system
	// that is identical to the local coordinate system, only with the y
	// axis pointing to the south instead of the north. This coordinate
	// system is called the "draw" coordinate system. We will later transform
	// the painter to this coordinate system.
	QTransform drawSystem_local;
	drawSystem_local.scale (1, -1);
	QTransform localSystem_draw=drawSystem_local.inverted ();
	// Calculate the transformation from the draw system to the widget system
	QTransform drawSystem_widget=drawSystem_local*localSystem_widget;

	// FIXME handle image.rotation
	foreach (const Image &image, images)
	{
		// Calculate the northwest and southeast corners of the image in the
		// local coordinate system
		QPointF northWest_local=image.northWest.relativePositionTo (_ownPosition);
		QPointF southEast_local=image.southEast.relativePositionTo (_ownPosition);

		// Calculate the northwest and southeast corners of the image in the
		// draw coordinate system (the painter will be transformed to this
		// coordinate system). These two corners define the rectangle the pixmap
		// will be drawn into.
		QPointF northWest_draw=northWest_local*localSystem_draw;
		QPointF southEast_draw=southEast_local*localSystem_draw;
		QRectF imageRect_draw (northWest_draw, southEast_draw);

		// Draw the whole pixmap in the draw coordinate system, into the
		// rectangle determined earlier.
		painter.setTransform (drawSystem_widget, false);
		painter.drawPixmap (imageRect_draw, image.pixmap, image.pixmap.rect ());
	}

	painter.restore ();
}

void FlarmMapWidget::paintCoordinateSystem (QPainter &painter)
{
	painter.save ();

	// Set a cosmetic pen (i. e. one that uses pixel dimensions, regardless of
	// the transformation) with a width of 0.5 pixels.
	QPen pen=painter.pen ();
	pen.setCosmetic (true);
	pen.setWidthF (1);
	painter.setPen (pen);

	double radiusIncrement=1000;

	// FIXME minimum pixel distance
	// FIXME draw all visible, even when the origin is scrolled out of view
	// FIXME draw distances
	for (double radius=radiusIncrement*0.01; radius<=_radius; radius+=radiusIncrement)
	{
		// FIXME inefficient
		QSizeF size (radius*width ()/getXRadius (), radius*height ()/getYRadius ());
		QPointF origin_local (0, 0); // The origin is at the origin of the local coordinate system
		QRectF rect=centeredQRectF (origin_local*localSystem_widget, size);
		painter.drawArc (rect, 0, 16*360);
	}

	// Draw an arrow from the own position to the north direction
	// In order to facilitate the arrow drawing, we transform the painter to FIXME.
	//	// FIXME arrow and "N"
	// Move the painter to a coordinate system that is centered at the own
	// position (the origin of the local system)
	painter.translate (QPointF (0, 0)*localSystem_widget);
	painter.scale (1, -1);
	painter.rotate (_orientation.toDegrees ());


	// FIXME ugly results
	int smallerSide=qMin (width (), height ());
	double len=smallerSide/4.0;
	painter.drawLine (QPointF (0, 0)       , QPointF (0, len));
	painter.drawLine (QPointF (-10, len-10), QPointF (0, len));
	painter.drawLine (QPointF ( 10, len-10), QPointF (0, len));

	QString text=tr (" N ");
	QSize size=textSize (painter, text);
	painter.translate (0, len+diameter (size)/2);
	painter.rotate (-_orientation.toDegrees ());
	painter.scale (1, -1);
	drawCenteredText (painter, QPointF (0, 0), text);


	painter.restore ();
}

void FlarmMapWidget::paintOwnPosition (QPainter &painter)
{
	painter.save ();
	// Draw the own position
	painter.setBrush (_ownPositionColor);
	// Draw at the own position
	QPointF position_local (0, 0);
	drawCenteredText (painter, position_local*localSystem_widget, _ownPositionText);
	painter.restore ();
}

void FlarmMapWidget::paintStaticCurves (QPainter &painter)
{
	if (!_ownPosition.isValid ())
		return;

	// Draw all static paths
	painter.save ();
	foreach (const StaticCurve &curve, staticCurves)
	{
		QPolygonF p=transformGeographicToWidget (curve.points);
		painter.setPen (curve.pen);
		painter.drawPolyline (p);
	}
	painter.restore ();
}

void FlarmMapWidget::paintStaticMarkers (QPainter &painter)
{
	if (!_ownPosition.isValid ())
		return;

	// Draw all static markers
	painter.save ();
	foreach (const StaticMarker &marker, staticMarkers)
	{
		QPointF p=transformGeographicToWidget (marker.position);
		painter.setBrush (marker.backgroundColor);
		drawCenteredText (painter, p, marker.text);
	}
	painter.restore ();
}

void FlarmMapWidget::paintPlanes (QPainter &painter)
{
	foreach (const PlaneMarker &marker, planeMarkers.values ())
	{
		QPointF position_widget=(marker.position_local*localSystem_widget).toPoint ();

		switch (marker.style)
		{
			case PlaneMarker::invisible:
				// Don't paint at all
				break;
			case PlaneMarker::minimal:
			{
				// Cross, 8 pixels in size
				drawOrthogonalCross (painter, position_widget, 4);
			} break;
			case PlaneMarker::verbose:
				// State-dependent text with state-dependent background color
				painter.setBrush (QBrush (marker.color));
				QPen pen;
				pen.setWidth (2);
				painter.setPen (pen);
				drawCenteredText (painter, position_widget, marker.text);
				painter.drawPolyline (marker.trail_local*localSystem_widget);
				break;
			// No default
		}

	}
}

/**
 * Paints the widget
 */
void FlarmMapWidget::paintEvent (QPaintEvent *event)
{
	// Paint the frame
	QFrame::paintEvent (event);

	// Make sure that all coordinate transforms are up to date
	updateTransforms ();

	// Create the painter and turn antialiasing on
	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing, true);

	// Paint the various items, in order from bottom to top. Note that some of
	// the items can only be painted if the own position is valid. For example,
	// static data is specified in absolute (earth) coordinates and the display
	// coordinate system is centered at the own position.
	painter.save (); paintImages           (painter); painter.restore ();
//	painter.save (); paintCoordinateSystem (painter); painter.restore ();
	painter.save (); paintOwnPosition      (painter); painter.restore ();
	painter.save (); paintStaticCurves     (painter); painter.restore ();
	painter.save (); paintStaticMarkers    (painter); painter.restore ();
	painter.save (); paintPlanes           (painter); painter.restore ();
}

