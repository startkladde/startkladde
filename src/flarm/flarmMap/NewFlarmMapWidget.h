#ifndef NEWFLARMMAPWIDGET_H_
#define NEWFLARMMAPWIDGET_H_

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

/*
 * Improvements over QwtPlot-based implementation:
 *   - visible while scrolling
 *   - simpler
 *   - shorter (?)
 *   - more versatile grid
 */
class NewFlarmMapWidget: public QFrame
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
			// FIME remove those that are not required
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

		NewFlarmMapWidget (QWidget *parent);
		virtual ~NewFlarmMapWidget ();

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
		void viewChanged ();
		void ownPositionUpdated ();

	protected:
		virtual void paintEvent (QPaintEvent *event);
		virtual void keyPressEvent (QKeyEvent *event);
		virtual void wheelEvent (QWheelEvent *event);
		virtual void mouseMoveEvent (QMouseEvent *event);
		virtual void mousePressEvent (QMouseEvent *event);
		virtual void mouseReleaseEvent (QMouseEvent *event);


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

		void paintCoordinateSystem (QPainter &painter);

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

	protected:
		virtual void updateView ();


	private:
		QPointF _center_local; // In local coordinates, because that's what stays fixed when rotating
		double _radius;
		Angle _orientation;

		// Basic transforms
		// _x means "in the x system"
		QTransform viewSystem_local, localSystem_view;
		QTransform plotSystem_view, viewSystem_plot;
		QTransform widgetSystem_plot, plotSystem_widget;

		// Combined transforms
		QTransform widgetSystem_local, localSystem_widget;

	private:
		bool scrollDragging;
		QPointF dragLocation_local;
		bool zoomDragging;
		double zoomDragStartRadius;
		QPoint zoomDragStartPosition_widget;

		int _keyboardZoomDoubleCount;
		double _mouseDragZoomDoubleDistance;
		Angle _mouseWheelZoomDoubleAngle;
};

#endif
