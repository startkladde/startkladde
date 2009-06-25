#ifndef WeatherDialog_h
#define WeatherDialog_h

/*
 * WeatherDialog
 * martin
 * 2008-02-18
 */
#include <QDialog>
#include <QLayout>
#include <QResizeEvent>

#include "src/gui/widgets/WeatherWidget.h"
#include "src/plugins/sk_plugin.h"

class WeatherDialog:public QDialog
{
	Q_OBJECT

	public:
		WeatherDialog (sk_plugin *_plugin, QWidget *parent=NULL, const char *name=NULL);
		~WeatherDialog ();

	protected:
		virtual void resizeEvent (QResizeEvent *);

	private:
		WeatherWidget *ww;
		sk_plugin *plugin;
		QHBoxLayout *weatherLayout;

};

#endif

