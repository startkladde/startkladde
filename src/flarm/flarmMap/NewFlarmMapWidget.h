#ifndef NEWFLARMMAPWIDGET_H_
#define NEWFLARMMAPWIDGET_H_

#include <QFrame>
#include <QString>
//#include <QTransform>
//#include <QVector>
//#include <QHash>

#include "src/flarm/flarmMap/Kml.h"
#include "src/numeric/GeoPosition.h"

class QModelIndex;

//class Angle;
//class FlarmRecord;
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


	public slots:
		void ownPositionChanged (const GeoPosition &ownPosition);
		void setOrientation (const Angle &upDirection);

	signals:
		void viewChanged ();
		void ownPositionUpdated ();

	protected:
		virtual void paintEvent (QPaintEvent *event);
		virtual void keyPressEvent (QKeyEvent *event);
		virtual void wheelEvent (QWheelEvent *event);

		virtual void zoom   (double factor);
		virtual void scroll (double x, double y);

	private:
		// Settings
		QColor _ownPositionColor, _climbColor, _descentColor;
		QString _ownPositionText;

		// Flarm list
		FlarmList *flarmList;
		GpsTracker *gpsTracker;

		// Coordinate systems:
		//   * earth coordinates: centered at the own position, coordinates are in meters

		// The current view
		// The view coordinate system is centered at the own position

		QPointF displayCenterPosition;
		double smallerRadius;
		QTransform transform;


		GeoPosition _ownPosition;

		// Static curves and Flarm data
//		QwtPlotMarker *ownPositionMarker;

		QList<StaticCurve> staticCurves;
		QList<StaticMarker> staticMarkers;

		QList<QPointF> allStaticPoints;
//		QHash<QString, QwtPlotMarker *> flarmMarkers;
//		QHash<QString, QwtPlotCurve  *> flarmCurves;
		GeoPosition kmlSouthEast, kmlNorthWest;

		// Status
		KmlStatus kmlStatus;

		// Static data
		void updateStaticData ();

		// Plot item updates
//		virtual void updateTrail (QwtPlotCurve *curve, const FlarmRecord &record);
//		virtual void updateMarkerMinimal (QwtPlotMarker *marker, const FlarmRecord &record);
//		virtual void updateMarkerVerbose (QwtPlotMarker *marker, const FlarmRecord &record);

		// Flarm data - Flarm list changes
//		virtual void addFlarmData (const FlarmRecord &record);
//		virtual void removeFlarmData (const FlarmRecord &record);
//		virtual void updateFlarmData (const FlarmRecord &record);
		virtual void refreshFlarmData ();

		// KML
		KmlStatus readKmlImplementation (const QString &filename);

		QPoint   mapFromGeographic (const GeoPosition &geoPosition);
		QPolygon mapFromGeographic (const QVector<GeoPosition> &geoPositions);
		QPoint   mapFromLocal      (const QPointF     &localPoint);
		QPolygon mapFromLocal      (const QPolygonF   &localPoints);

		void paintCoordinateSystem (QPainter &painter);

	protected:
		// Generic axis methods
		QRectF getAxesRect () const;
		QPointF getAxesRadius () const;
		QPointF getAxesCenter () const;
		void setAxes (const QPointF &center, const QPointF &radius);
		void setAxesRadius (const QPointF &radius);
		void setAxesRadius (double xRadius, double yRadius);
		void setAxesCenter (const QPointF &center);
		void zoomAxes (double factor);
		void moveAxesCenter (const QPointF &offset);
		void moveAxesCenter (double xOffset, double yOffset);

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
};

#endif
