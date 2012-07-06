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
 * Draws a map containing static curves (like the airfield outline or the
 * traffic circuit) and markers as well as dynamic Flarm data, read from a model
 *
 * For planes on the ground, a small cross is drawn. For flying planes, a label
 * with the registration, indicating (relative) altitude, airspeed and climb
 * rate is shown. Additionally, a trail indicating the path of the plane during
 * the last couple of seconds is drawn.
 *
 * The own position is always at the origin of the coordinate system. The static
 * data is drawn whenever the own position is available. The Flarm data is based
 * on relative positions and will be drawn even if the own position is unknown.
 *
 * This widget provides scrolling (by left-dragging with the mouse) and zooming
 * (by mouse wheel or middle-dragging) functionality.
 *
 * Note that the Flarm data is based on a relative position. When the own
 * position changes, the past Flarm data is not updated accordingly. Therefore,
 * while the current Flarm positions will always be correct, the trail will be
 * wrong when the own position changes rapidly. However, this will not be the
 * case in a typical usage scenario.
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

		// View
		void setOrientation (const Angle &upDirection);

	private:
		// Settings
		QColor climbColor, descentColor;

		// Model
		FlarmList *model;

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
		void ownPositionChanged (const GeoPosition &ownPosition);

		// Model slots
		void rowsInserted (const QModelIndex &parent, int start, int end);
		void dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		void rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		void modelReset ();
		void modelDestroyed ();
};

#endif
