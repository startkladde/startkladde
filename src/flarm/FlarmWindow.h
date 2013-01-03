#ifndef FLARM_WINDOW_H
#define FLARM_WINDOW_H

#include "ui_FlarmWindow.h"

#include "src/gui/SkDialog.h"

class FlarmList;
class GpsTracker;
class QStatusBar;

class FlarmWindow: public SkDialog<Ui::FlarmWindowClass>
{
	Q_OBJECT

	public:
		FlarmWindow (QWidget *parent);
		~FlarmWindow ();

		void setGpsTracker (GpsTracker *gpsTracker);
		void setFlarmList (FlarmList *flarmList);

	public slots:
		void flarmMapViewChanged ();
		void flarmMapOwnPositionUpdated ();

	private slots:
		void on_mapOrientationInput_valueChanged (int value);
		void on_northButton_clicked ();
		void on_reverseButton_clicked ();
		void on_flarmMap_mouseMoved_p (QPointF position_p);
		void on_flarmMap_mouseLeft ();
		void on_flarmMap_orientationChanged ();

		void linkActivated (const QString &link);


		void updateWarnings ();

	private:
		GpsTracker *gpsTracker;
		QStatusBar *statusBar;
		QLabel *mousePositionLabel;
};

#endif
