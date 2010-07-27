#ifndef LONGITUDEINPUT_H
#define LONGITUDEINPUT_H

#include <QtGui/QWidget>
#include "ui_LongitudeInput.h"

#include "src/Longitude.h"

class LongitudeInput: public QWidget
{
		Q_OBJECT

	public:
		LongitudeInput (QWidget *parent=NULL);
		~LongitudeInput ();

		void setLongitude (const Longitude &longitude);
		Longitude getLongitude () const;

	private:
		Ui::LongitudeInputClass ui;
};

#endif
