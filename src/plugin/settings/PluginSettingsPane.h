#ifndef PLUGINSETTINGSPANE_H_
#define PLUGINSETTINGSPANE_H_

#include <QtGui/QWidget>

class PluginSettingsPane: public QWidget
{
	public:
		PluginSettingsPane (QWidget *parent=NULL);
		virtual ~PluginSettingsPane ();

	public slots:
		virtual void readSettings ()=0;
		virtual void writeSettings ()=0;
};

#endif
