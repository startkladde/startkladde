#ifndef FLARM_MAP_H
#define FLARM_MAP_H

#include "ui_FlarmMap.h"

#include "src/gui/SkDialog.h"

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

	private slots:
		void on_mapOrientationInput_valueChanged (int value);
		void on_northButton_clicked ();
		void on_reverseButton_clicked ();
};

#endif
