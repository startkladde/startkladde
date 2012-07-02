#ifndef FLARM_MAP_H
#define FLARM_MAP_H

#include "ui_FlarmMap.h"

#include "src/gui/SkDialog.h"
#include "src/numeric/GeoPosition.h"

class QSettings;
class QTransform;

class QwtPlotMarker;
class QwtPlotCurve;
class QwtPointSeriesData;

class FlarmMap: public SkDialog<Ui::FlarmMapDialog>
{
	Q_OBJECT

	public:
		FlarmMap (QWidget *parent);
		~FlarmMap ();

	private:
		void readVectors ();
		void setExampleVectors ();
		void storeVectors ();

		void redrawFlarmData ();
		void redrawStaticData ();

		void setOrientation (const Angle &upDirection);

		QTransform transform;

		// Static data
		QVector<GeoPosition> airfieldVector, patternVector;
		QwtPlotCurve *airfieldCurve, *patternCurve;
		QwtPointSeriesData *airfieldData, *patternData;
		GeoPosition ownPosition;

		QList<QwtPlotMarker *> flarmMarkers;
		QList<QwtPlotCurve *> flarmCurves;

	private slots:
		void flarmStatusChanged ();
		void ownPositionChanged (const GeoPosition &ownPosition);

		void on_mapOrientationInput_valueChanged (int value);
		void on_northButton_clicked ();
		void on_reverseButton_clicked ();

};

#endif
