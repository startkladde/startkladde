#ifndef FLARMMAPWIDGET_H_
#define FLARMMAPWIDGET_H_

#include <QFrame>
#include <QString>

#include "src/flarm/flarmMap/Kml.h"
#include "src/numeric/GeoPosition.h"
#include "src/numeric/Angle.h"

class QModelIndex;

//class Angle;
class FlarmRecord;
class FlarmList;
class GpsTracker;
class QResizeEvent;

/*
 * Improvements over QwtPlot-based implementation:
 *   - visible while scrolling
 *   - simpler
 *   - shorter (?)
 *   - more versatile grid
 */
class FlarmMapWidget: public QFrame
{
		Q_OBJECT

	public:
		enum KmlStatus { kmlNone, kmlNotFound, kmlReadError, kmlParseError, kmlEmpty, kmlOk };

		struct StaticCurve
		{
			StaticCurve (const QVector<GeoPosition> &points, const QString &name, const QPen &pen);
			StaticCurve (const Kml::Path    &path   , const Kml::Style &style);
			StaticCurve (const Kml::Polygon &polygon, const Kml::Style &style);
			QVector<GeoPosition> points;
			QString name;
			QPen pen;
		};

		struct StaticMarker
		{
			StaticMarker (const GeoPosition &position, const QString &text, const QColor &backgroundColor);
			StaticMarker (const Kml::Marker &marker, const Kml::Style &style);
			GeoPosition position;
			QString text;
			QColor backgroundColor;
		};

		struct Image
		{
			Image (const Kml::GroundOverlay &groundOverlay);

			QPixmap pixmap;
			GeoPosition northEast, southWest;
			GeoPosition northWest, southEast;
		};

		struct PlaneMarker
		{
			enum Style { invisible, minimal, verbose };

			Style style;
			QPointF position_local;
			QString text;
			QPolygonF trail_local;
			QColor color;
		};

		FlarmMapWidget (QWidget *parent);
		virtual ~FlarmMapWidget ();

		// Static data
		void setOwnPositionLabel (const QString &text, const QColor &color);

		// KML
		KmlStatus readKml (const QString &filename);
		KmlStatus getKmlStatus () const;

		// Flarm list
		void setFlarmList (FlarmList *flarmList);
		void setGpsTracker (GpsTracker *gpsTracker);

		// Status
		bool isOwnPositionKnown () const;

		// View
		bool isOwnPositionVisible () const;
		bool isAnyStaticElementVisible () const;
		bool findClosestStaticElement (double *distance, Angle *bearing) const;
		void resetPosition ();


	signals:
		void viewChanged () const;
		void ownPositionUpdated () const;
		void orientationChanged () const;
		void mouseMoved (QPointF position_local) const;
		void mouseLeft () const;

		// ***** Painting
	protected:
		virtual void paintEvent (QPaintEvent *event);
		virtual void paintCoordinateSystem (QPainter &painter);
		virtual void paintStaticCurves (QPainter &painter);
		virtual void paintStaticMarkers (QPainter &painter);
		virtual void paintImages (QPainter &painter);
		virtual void paintPlanes (QPainter &painter);
		virtual void paintOwnPosition (QPainter &painter);



		virtual void keyPressEvent (QKeyEvent *event);
		virtual void wheelEvent (QWheelEvent *event);
		virtual void mouseMoveEvent (QMouseEvent *event);
		virtual void mousePressEvent (QMouseEvent *event);
		virtual void mouseReleaseEvent (QMouseEvent *event);
		virtual void resizeEvent (QResizeEvent *event);
		virtual void leaveEvent (QEvent *event);


	private:
		// Settings
		QColor _ownPositionColor, _climbColor, _descentColor;
		QString _ownPositionText;

		// Flarm list
		FlarmList *flarmList;
		GpsTracker *gpsTracker;

		GeoPosition _ownPosition;

		// Static curves and Flarm data
		QList<StaticCurve> staticCurves;
		QList<StaticMarker> staticMarkers;
		QList<Image> images;

		QList<GeoPosition> allStaticPositions;
		QHash<QString, PlaneMarker> planeMarkers;
		GeoPosition kmlSouthEast, kmlNorthWest;

		// Status
		KmlStatus kmlStatus;

		// Static data
		void updateStaticData ();

		// Flarm data - Flarm list changes
		virtual void addPlaneMarker (const FlarmRecord &record);
		virtual void updatePlaneMarker (const FlarmRecord &record);
		virtual void removePlaneMarker (const FlarmRecord &record);
		virtual void refreshPlaneMarkers ();

		// KML
		KmlStatus readKmlImplementation (const QString &filename);

		QPointF   transformGeographicToWidget (const GeoPosition &geoPosition) const;
		QPolygonF transformGeographicToWidget (const QVector<GeoPosition> &geoPositions) const;


	protected:
		double getLargerRadius () const;
		double getXRadius () const;
		double getYRadius () const;

	private slots:
		// FlarmList model slots
		void rowsInserted (const QModelIndex &parent, int start, int end);
		void dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		void rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		void modelReset ();
		void flarmListDestroyed ();



		// View

	public slots:
		virtual void ownPositionChanged (const GeoPosition &ownPosition);
		virtual void setOrientation (const Angle &orientation);
		virtual void zoom   (double factor);
		virtual void scroll (double x, double y);

		virtual Angle orientation ();

	protected:
		virtual void updateView ();


	private:
		QPointF _center_local; // In local coordinates, because that's what stays fixed when rotating
		double _radius;
		Angle _orientation;

		// Basic transforms
		// _x means "in the x system"
		mutable bool transformsValid;
		void invalidateTransforms ();
		void updateTransforms () const;
		// Individual transforms
		mutable QTransform viewSystem_local, localSystem_view;
		mutable QTransform plotSystem_view, viewSystem_plot;
		mutable QTransform widgetSystem_plot, plotSystem_widget;
		// Combined transforms
		mutable QTransform widgetSystem_local, localSystem_widget;

	private:
		// Scroll dragging
		bool scrollDragging;
		QPointF dragLocation_local;

		// Zoom dragging
		bool zoomDragging;
		double zoomDragStartRadius;
		QPoint zoomDragStartPosition_widget;

		// Zoom rotation
		bool rotateDragging;
		Angle rotateDragStartOrientation;
		QPoint rotateDragStartPosition_widget;

		// Zoom/rotation factors
		int _keyboardZoomDoubleCount;
		double _mouseDragZoomDoubleDistance;
		Angle _mouseWheelZoomDoubleAngle;
		double _mouseDragRotationDistance;

};

#endif
