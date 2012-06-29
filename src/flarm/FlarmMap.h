#ifndef FLARM_MAP_H
#define FLARM_MAP_H

#include "ui_FlarmMap.h"

#include "src/gui/SkDialog.h"
#include "src/numeric/GeoPosition.h"

class QwtPointSeriesData;
class QSettings;

//#include <QtGui/QWidget>
//#include <QtGui/QDialog>


class FlarmMap: public SkDialog<Ui::FlarmMapDialog>
{
	Q_OBJECT

	public:
		FlarmMap (QWidget *parent);
		~FlarmMap ();

	private slots:
		void refreshFlarm ();
		void storeVectors ();
		void readVectors ();

		void drawAirfield (const GeoPosition &home);

	private slots:
		void on_toggleOrientationButton_toggled (bool on);

	private:
		void setExampleVectors ();

		QVector<GeoPosition> airfieldVector;
		QVector<GeoPosition> patternVector;

		QwtPointSeriesData *data;
		GeoPosition oldHome;
		double northUp;
};

#endif
