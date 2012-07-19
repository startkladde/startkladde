#ifndef FLARM_WINDOW_H
#define FLARM_WINDOW_H

#include "ui_FlarmWindow.h"

#include "src/gui/SkDialog.h"

class FlarmWindow: public SkDialog<Ui::FlarmWindowClass>
{
	Q_OBJECT

	public:
		FlarmWindow (QWidget *parent);
		~FlarmWindow ();

		void setGpsTracker (GpsTracker *gpsTracker);
		void setFlarmList (FlarmList *flarmList);

	private slots:
		void on_mapOrientationInput_valueChanged (int value);
		void on_northButton_clicked ();
		void on_reverseButton_clicked ();
};

#endif
