#ifndef _WeatherDialog_h
#define _WeatherDialog_h

#include <QDialog>
#include <QLayout>
#include <QResizeEvent>

#include "src/gui/widgets/WeatherWidget.h"
#include "src/plugins/ShellPlugin.h"

class WeatherDialog:public QDialog
{
	Q_OBJECT

	public:
		WeatherDialog (ShellPlugin *_plugin, QWidget *parent=NULL);
		~WeatherDialog ();

	protected:
		virtual void resizeEvent (QResizeEvent *);

	private:
		WeatherWidget *ww;
		ShellPlugin *plugin;
		QHBoxLayout *weatherLayout;

};

#endif

