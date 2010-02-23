#ifndef WEATHERDIALOG_H_
#define WEATHERDIALOG_H_

#include <QDialog>

class QResizeEvent;
class QHBoxLayout;

class ShellPlugin;
class WeatherWidget;

class WeatherDialog:public QDialog
{
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

