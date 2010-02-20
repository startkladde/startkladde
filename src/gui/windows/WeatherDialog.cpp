#include "WeatherDialog.h"

#include <QLayout>

#include "src/gui/widgets/WeatherWidget.h"
#include "src/plugins/ShellPlugin.h"

WeatherDialog::WeatherDialog (ShellPlugin *_plugin, QWidget *parent)
	:QDialog (parent)
{
	plugin=_plugin;

	setAttribute (Qt::WA_DeleteOnClose, true);

	ww=new WeatherWidget (this, "ww");

	QObject::connect (plugin, SIGNAL (lineRead (QString)), ww, SLOT (inputLine (QString)));
	QObject::connect (plugin, SIGNAL (pluginNotFound ()), ww, SLOT (pluginNotFound ()));
	QObject::connect (ww, SIGNAL (doubleClicked ()), plugin, SLOT (restart ()));
	plugin->start ();

	weatherLayout=new QHBoxLayout ();
	weatherLayout->setSizeConstraint (QLayout::SetFixedSize);
	weatherLayout->addWidget (ww);
	setLayout (weatherLayout);
}

WeatherDialog::~WeatherDialog ()
{
	delete plugin;
}

void WeatherDialog::resizeEvent (QResizeEvent *e)
{
	(void)e;
//	// Bei Größenänderung: so verschieben, dass das Zentrum des Dialogs an der
//	// gleichen Stelle bleibt.
//
//	// So oder so?
//	//int oldCenterX=geometry ().x ()+e->oldSize ().width ()/2;
//	//int oldCenterY=geometry ().y ()+e->oldSize ().height ()/2;
//	int oldCenterX=x ()+e->oldSize ().width ()/2;
//	int oldCenterY=y ()+e->oldSize ().height ()/2;
//
//	int newX=oldCenterX-frameGeometry ().width ()/2;
//	int newY=oldCenterY-frameGeometry ().height ()/2;
//
//	move (newX, newY);
}

