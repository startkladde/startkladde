#include "WeatherDialog.h"

#include <QtGui/QMovie>

WeatherDialog::WeatherDialog (sk_plugin *_plugin, QWidget *parent, const char *name)/*{{{*/
	:QDialog (parent, name)
{
	plugin=_plugin;

	ww=new weather_widget (this, "ww");

	QObject::connect (plugin, SIGNAL (lineRead (QString)), ww, SLOT (inputLine (QString)));
	QObject::connect (plugin, SIGNAL (pluginNotFound ()), ww, SLOT (pluginNotFound ()));
	QObject::connect (ww, SIGNAL (doubleClicked ()), plugin, SLOT (restart ()));
	plugin->start ();

	weatherLayout=new QHBoxLayout ();
	weatherLayout->setSizeConstraint (QLayout::SetFixedSize);
	weatherLayout->addWidget (ww);
	setLayout (weatherLayout);
}/*}}}*/

WeatherDialog::~WeatherDialog ()/*{{{*/
{
	if (plugin) delete plugin; plugin=NULL;
}
/*}}}*/

void WeatherDialog::resizeEvent (QResizeEvent *e)/*{{{*/
{
	// Bei Größenänderung: so verschieben, dass das Zentrum des Dialogs an der
	// gleichen Stelle bleibt.

	// So oder so?
	//int oldCenterX=geometry ().x ()+e->oldSize ().width ()/2;
	//int oldCenterY=geometry ().y ()+e->oldSize ().height ()/2;
	int oldCenterX=x ()+e->oldSize ().width ()/2;
	int oldCenterY=y ()+e->oldSize ().height ()/2;

	int newX=oldCenterX-frameGeometry ().width ()/2;
	int newY=oldCenterY-frameGeometry ().height ()/2;

	move (newX, newY);
}
/*}}}*/

