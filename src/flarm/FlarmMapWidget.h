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
 *   modelDestroyed
 *       setModel
 */

/**
 * A widget that shows a map containing static curves (like the airfield outline
 * or the traffic circuit), static markers (e. g. landmarks or the own position)
 * and dynamic Flarm data (read from a model)
 *
 * This widget acts as a view for the FlarmList model, using the QAbstractItemList
 * interface. It provides scrolling (by left-dragging with the mouse) and zooming
 * (by mouse wheel or middle-dragging) functionality.
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
 * To use this widget, set the model to show by calling setModel. Call
 * ownPositionChanged whenever the own (receiver) position changes. You can add
 * one or more static curves or markers at any time by calling addStaticCurve or
 * addStaticMarker, respectively.
 *
 *
 * == Implementation details ==
 *
 * === Plotting ===
 *
 * FIXME document
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
class FlarmMapWidget: public QwtPlot
{
		Q_OBJECT

	public:
		struct StaticCurve
		{
			QString name;
			QVector<GeoPosition> points;
			QwtPlotCurve *curve;
			QwtPointSeriesData *data;
		};

		FlarmMapWidget (QWidget *parent=0);
		virtual ~FlarmMapWidget ();

		// Static data
		void addStaticCurve (const QString &name, const QVector<GeoPosition> &points, QPen pen);
		void addStaticMarker (const QString &text, const QColor &color, const QPointF &point);

		// Model
		void setModel (FlarmList *model);
		void setGpsTracker (GpsTracker *gpsTracker);

	public slots:
		void ownPositionChanged (const GeoPosition &ownPosition);
		void setOrientation (const Angle &upDirection);

	private:
		// Settings
		QColor climbColor, descentColor;

		// Model
		FlarmList *model; // FIXME rename flarmList
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

		// Flarm data - model changes
		virtual void addFlarmData (const FlarmRecord &record);
		virtual void removeFlarmData (const FlarmRecord &record);
		virtual void updateFlarmData (const FlarmRecord &record);
		virtual void refreshFlarmData ();

	private slots:
		// Model slots
		void rowsInserted (const QModelIndex &parent, int start, int end);
		void dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		void rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		void modelReset ();
		void modelDestroyed ();
};

#endif
