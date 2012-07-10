#ifndef FLARMMAPWIDGET_H_
#define FLARMMAPWIDGET_H_

#include <QTransform>
#include <QVector>
#include <QHash>

#include <qwt_plot.h>

#include "src/numeric/GeoPosition.h"

class QModelIndex;

class QwtPlotMarker;
class QwtPlotCurve;
class QwtPointSeriesData;

class Angle;
class FlarmRecord;
class FlarmList;
class GpsTracker;

/*
 * Call tree:
 *   rowsInserted
 *       addFlarmData
 *   dataChanged
 *       updateFlarmData
 *           updateTrail
 *           updateMarkerMinimal
 *           updateMarkerVerbose
 *   rowsAboutToBeRemoved
 *       removeFlarmData
 *   modelReset
 *       refreshFlarmData
 *           addFlarmData
 *   flarmListDestroyed
 *       modelReset
 *           ...
 */

/**
 * A widget that shows a map containing static curves (like the airfield outline
 * or the traffic circuit), static markers (e. g. landmarks or the own position)
 * and dynamic Flarm data (read from a model)
 *
 * This widget acts as a view for the FlarmList model, using its
 * QAbstractItemList interface. It provides scrolling (by left-dragging with the
 * mouse) and zooming (by mouse wheel or middle-dragging) functionality.
 *
 * Planes (Flarm records) are drawn in different ways, depending on their state:
 * planes on the ground are drawn as a small cross. Flying planes are
 * represented as a label  showing the registration, (relative) altitude,
 * airspeed and climb rate. Additionally, a trail indicating the path of the
 * plane over the last couple of seconds is drawn.
 *
 * The own position is always at the origin of the coordinate system. The static
 * data is drawn whenever the own position is available. The Flarm data is based
 * on relative positions and will be drawn even if the own position is unknown.
 *
 * Note that the Flarm data is based on a relative position. When the own
 * position changes, the past Flarm data is not updated accordingly. Therefore,
 * the trail will be wrong (although the current Flarm positions will always be
 * correct to within the update interval, which is typically one second). In a
 * typical usage scenario, however, the own position will not change
 * significantly.
 *
 * To use this widget, set the Flarm List to show by calling setFlarmList. Call
 * ownPositionChanged whenever the own (receiver) position changes. You can add
 * one or more static curves or markers at any time by calling addStaticCurve or
 * addStaticMarker, respectively.
 *
 *
 * == Implementation details ==
 *
 * === Plotting ===
 *
 * Markers (static markers and Flarm markers) are stored as QwtPlotMarker*.
 * There position ("value") is stored directly in the QwtPlotMarker. Curves
 * (static curves and Flarm trails) are stored as QwtPlotCurve*. Their points
 * are stored in a QwtPointSeriesData*.
 *
 * When the orientation of the map or dynamic data (own position or Flarm data)
 * changes, the points are updated. For the changes to be visible, replot() must
 * be called. Since replot() is slow and typically, several points change at the
 * same time, must functions which change points do not call replot() themselves
 * but leave that to the caller.
 *
 * FIXME: documented in function documentation, which ones do/don't
 *
 *
 * === Flarm data ===
 *
 * FIXME document
 *
 * === Static data ===
 *
 * FIXME document
 *
 *
 */
// FIXME on setting the GPS tracker, get the position
class FlarmMapWidget: public QwtPlot
{
		Q_OBJECT

	public:
		struct StaticCurve
		{
			QString name;
			QVector<GeoPosition> points;
			QwtPlotCurve *curve;
			// FIXME why do we store it explicitly for static curves but not for
			// Flarm curves?
			QwtPointSeriesData *data;
		};

		FlarmMapWidget (QWidget *parent=0);
		virtual ~FlarmMapWidget ();

		// Static data
		void addStaticCurve (const QString &name, const QVector<GeoPosition> &points, QPen pen);
		void addStaticMarker (const QString &text, const QColor &color, const QPointF &point);

		// Flarm list
		void setFlarmList (FlarmList *flarmList);
		void setGpsTracker (GpsTracker *gpsTracker);

	public slots:
		void ownPositionChanged (const GeoPosition &ownPosition);
		void setOrientation (const Angle &upDirection);

	protected:
		virtual void resizeEvent (QResizeEvent *event);

	private:
		// Settings
		QColor climbColor, descentColor;

		// Flarm list
		FlarmList *flarmList;
		GpsTracker *gpsTracker;

		// View settings
		QTransform transform;
		GeoPosition ownPosition;

		// Static curves and Flarm data
		QList<StaticCurve> staticCurves;
		QHash<QString, QwtPlotMarker *> flarmMarkers;
		QHash<QString, QwtPlotCurve  *> flarmCurves;


		// Static data
		void updateStaticCurves ();

		// Plot item updates
		virtual void updateTrail (QwtPlotCurve *curve, const FlarmRecord &record);
		virtual void updateMarkerMinimal (QwtPlotMarker *marker, const FlarmRecord &record);
		virtual void updateMarkerVerbose (QwtPlotMarker *marker, const FlarmRecord &record);

		// Flarm data - Flarm list changes
		virtual void addFlarmData (const FlarmRecord &record);
		virtual void removeFlarmData (const FlarmRecord &record);
		virtual void updateFlarmData (const FlarmRecord &record);
		virtual void refreshFlarmData ();

	protected:
		// Generic axis methods
		QPointF getAxesRadius () const;
		QPointF getAxesCenter () const;
		void setAxes (const QPointF &center, const QPointF &radius);
		void setAxesRadius (const QPointF &radius);
		void setAxesRadius (double xRadius, double yRadius);

	private slots:
		// FlarmList model slots
		void rowsInserted (const QModelIndex &parent, int start, int end);
		void dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		void rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		void modelReset ();
		void flarmListDestroyed ();
};

#endif
