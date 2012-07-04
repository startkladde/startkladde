#ifndef FLARMMAPWIDGET_H_
#define FLARMMAPWIDGET_H_

#include <QTransform>
#include <QVector>
#include <QModelIndex>

#include <qwt_plot.h>

#include "src/numeric/GeoPosition.h"

class QwtPlotMarker;
class QwtPlotCurve;
class QwtPointSeriesData;

class Angle;
class FlarmRecord;
class FlarmList;

class FlarmMapWidget: public QwtPlot
{
		Q_OBJECT

	public:
		FlarmMapWidget (QWidget *parent=0);
		virtual ~FlarmMapWidget ();

		void setFlarmList (FlarmList *list);

		void setAirfieldVector (const QVector<GeoPosition> &vector);
		void setPatternVector (const QVector<GeoPosition> &vector);
		QVector<GeoPosition> getAirfieldVector () const;
		QVector<GeoPosition> getPatternVector () const;

		void setOrientation (const Angle &upDirection);

	protected:
		void addMinimalPlaneMarker (const FlarmRecord *record);
		void addVerbosePlaneMarker (const FlarmRecord *record);
		void addTrail (const FlarmRecord *record);

	private:
		void redrawFlarmData ();
		void redrawStaticData ();


		QTransform transform;

		// Static data
		QVector<GeoPosition> airfieldVector, patternVector;
		QwtPlotCurve *airfieldCurve, *patternCurve;
		QwtPointSeriesData *airfieldData, *patternData;
		GeoPosition ownPosition;

		QList<QwtPlotMarker *> flarmMarkers;
		QList<QwtPlotCurve *> flarmCurves;

		// Flarm list
		FlarmList *flarmList;

	private slots:
		void ownPositionChanged (const GeoPosition &ownPosition);

		void flarmListDestroyed ();
		void flarmListDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		void flarmListRowsInserted (const QModelIndex &parent, int start, int end);
		void flarmListRowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		void flarmListReset ();
		void flarmListLayoutChanged ();
};

#endif
