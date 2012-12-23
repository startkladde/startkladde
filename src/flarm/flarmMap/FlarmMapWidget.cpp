#include "src/flarm/flarmMap/FlarmMapWidget.h"

//#include <cassert>
#include <iostream>

#include <QModelIndex>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>

// FIXME: must emit viewChanged, and only when appropriate

#include "src/util/qRect.h"
#include "src/flarm/FlarmRecord.h"
#include "src/numeric/Velocity.h"
#include "src/flarm/FlarmList.h"
//#include "src/util/qHash.h"
#include "src/i18n/notr.h"
#include "src/nmea/GpsTracker.h"
#include "src/util/qPointF.h"
//#include "src/util/qString.h"
#include "src/flarm/flarmMap/KmlReader.h"
#include "src/util/qPainter.h"


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
	_center_local (0, 0), _radius (2000),
	scrollDragging (false), zoomDragging (false),
	_keyboardZoomDoubleCount (8), _mouseDragZoomDoubleDistance (50),
	_mouseWheelZoomDoubleAngle (Angle::fromDegrees (120))
{
	_ownPositionText=tr ("Start"); // FIXME proper English word
}

FlarmMapWidget::~FlarmMapWidget ()
{
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
	// FIXME use zoom(), and zoom around the mouse wheel position
	// Mouse wheel down (back) means zooming out. This is the convention that
	// many other applications, including Firefox and Gimp, use.
	Angle angle=Angle::fromDegrees (event->delta ()/(double)8);
	_radius*=pow (2, -angle/_mouseWheelZoomDoubleAngle);
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
	_orientation=orientation;

	// Schedule a repaint
	update ();
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
		update ();
		emit ownPositionUpdated (); // FIXME what for?
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
		// FIXME allStaticPositions
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


// **************
// ** Painting **
// **************

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

void FlarmMapWidget::paintCoordinateSystem (QPainter &painter)
{
	painter.save ();
	QPen pen=painter.pen ();
	pen.setCosmetic (true);
	pen.setWidthF (0.5);
	painter.setPen (pen);

	double radiusIncrement=1000;

	// FIXME minimum pixel distance
	// FIXME draw all visible, even when the origin is scrolled out of view
	// FIXME draw distances
	for (double radius=radiusIncrement; radius<=_radius; radius+=radiusIncrement)
	{
		// FIXME inefficient
		QSizeF size (radius*width ()/getXRadius (), radius*height ()/getYRadius ());
		QPointF origin_local (0, 0); // The origin is at the origin of the local coordinate system
		QRectF rect=centeredQRectF (origin_local*localSystem_widget, size);
		painter.drawArc (rect, 0, 16*360);
	}

	// Draw an arrow from the own position to the north direction
	// In order to facilitate the arrow drawing, we transform the painter.
	painter.setTransform (localSystem_widget);
	painter.drawLine (0, 0, 0, 500);


	// FIXME arrow and "N"
	QTransform transform; transform.rotateRadians (_orientation.toRadians ());
	painter.setTransform (transform.transposed (), true);
	painter.drawLine (0, 0, 0, -qMin (width (), height ())*1/4);

	painter.restore ();
}

void FlarmMapWidget::paintEvent (QPaintEvent *event)
{
	// FIXME only when required
	updateView ();

	(void)event;
	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing, true);

	painter.setPen (Qt::black);

	if (_ownPosition.isValid ())
	{
		painter.save ();

		// FIXME doing handle north/south/east/west
		// FIXME handle image.rotation
		foreach (const Image &image, images)
		{
			QPointF northEast_local=image.northEast.relativePositionTo (_ownPosition);
			QPointF southWest_local=image.southWest.relativePositionTo (_ownPosition);
			QPointF northWest_local=image.northWest.relativePositionTo (_ownPosition);
			QPointF southEast_local=image.southEast.relativePositionTo (_ownPosition);

			QTransform drawSystem_local;
			drawSystem_local.scale (1, -1);
			QTransform localSystem_draw=drawSystem_local.inverted ();

			QTransform drawSystem_widget=drawSystem_local*localSystem_widget;

			QPointF northWest_draw=northWest_local*localSystem_draw;
			QPointF southEast_draw=southEast_local*localSystem_draw;
			QRectF imageRect_draw (northWest_draw, southEast_draw);

			painter.setTransform (drawSystem_widget, false);
			painter.drawPixmap (imageRect_draw, image.pixmap, image.pixmap.rect ());
		}

		painter.restore ();
	}

	painter.save ();
	paintCoordinateSystem (painter);
	painter.restore ();

	painter.save ();
	// Draw the own position
	painter.setBrush (_ownPositionColor);
	// Draw at the own position
	QPointF position_local (0, 0);
	drawCenteredText (painter, position_local*localSystem_widget, _ownPositionText);
	painter.restore ();

	// We can only draw the static data if the own position is known, because it
	// is specified in absolute (earth) coordinates and the display coordinate
	// system is centered at the own position.
	painter.save ();
	if (_ownPosition.isValid ())
	{
		// Draw all static paths
		foreach (const StaticCurve &curve, staticCurves)
		{
			QPolygonF p=transformGeographicToWidget (curve.points);
			painter.setPen (curve.pen);
			painter.drawPolyline (p);
		}

		// Draw all static markers
		foreach (const StaticMarker &marker, staticMarkers)
		{
			QPointF p=transformGeographicToWidget (marker.position);
			painter.setBrush (marker.backgroundColor);
			drawCenteredText (painter, p, marker.text);
		}
	}
	painter.restore ();

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
				QPointF dx (4, 0);
				QPointF dy (0, 4);
				painter.drawLine (position_widget-dx, position_widget+dx);
				painter.drawLine (position_widget-dy, position_widget+dy);
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

void FlarmMapWidget::mousePressEvent (QMouseEvent *event)
{
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
	}
}

void FlarmMapWidget::mouseReleaseEvent (QMouseEvent *event)
{
	if (event->button ()==Qt::LeftButton)
		scrollDragging=false;
	else if (event->button ()==Qt::RightButton)
		zoomDragging=false;
}

void FlarmMapWidget::mouseMoveEvent (QMouseEvent *event)
{
	if (scrollDragging)
	{
		QPointF mousePosition_widget=event->posF ();

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
		int delta=event->pos ().y () - zoomDragStartPosition_widget.y ();
		_radius=zoomDragStartRadius*pow (2, delta/_mouseDragZoomDoubleDistance);
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
	update ();
}

/**
 * Undefined if the own position is invalid
 */
QPointF FlarmMapWidget::transformGeographicToWidget (const GeoPosition &geoPosition) const
{
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


