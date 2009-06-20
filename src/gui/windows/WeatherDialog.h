#ifndef WeatherDialog_h
#define WeatherDialog_h

/*
 * WeatherDialog
 * martin
 * 2008-02-18
 */
#include <QtGui/QDialog>
#include <QtGui/QLayout>
#include <QtGui/QResizeEvent>

#include "src/gui/widgets/weather_widget.h"
#include "src/plugins/sk_plugin.h"

using namespace std;

class WeatherDialog:public QDialog
{
	Q_OBJECT

	public:
		WeatherDialog (sk_plugin *_plugin, QWidget *parent=NULL, const char *name=NULL);
		~WeatherDialog ();

	protected:
		virtual void resizeEvent (QResizeEvent *);

	private:
		weather_widget *ww;
		sk_plugin *plugin;
		QHBoxLayout *weatherLayout;

};

#endif

