#include "LongitudeInput.h"

LongitudeInput::LongitudeInput (QWidget *parent):
	QWidget (parent)
{
	ui.setupUi (this);

	setFocusProxy (ui.degreesInput);
}

LongitudeInput::~LongitudeInput ()
{

}

void LongitudeInput::setLongitude (const Longitude &longitude)
{
	Longitude lon=longitude.normalized ();
	ui.degreesInput->setValue (lon.getDegrees ());
	ui.minutesInput->setValue (lon.getMinutes ());
	ui.secondsInput->setValue (lon.getSeconds ());
	ui.signInput->setCurrentIndex (lon.isPositive ()?0:1);
}

Longitude LongitudeInput::getLongitude () const
{
	return Longitude (
		ui.degreesInput->value (),
		ui.minutesInput->value (),
		ui.secondsInput->value (),
		ui.signInput->currentIndex ()==0
		);
}
