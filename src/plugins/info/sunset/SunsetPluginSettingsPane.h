#ifndef SunsetPluginSETTINGSPANE_H
#define SunsetPluginSETTINGSPANE_H

#include "src/plugin/settings/PluginSettingsPane.h"
#include "ui_SunsetPluginSettingsPane.h"

// FIXME forward declaration sufficient? (also other plugins)
#include "src/plugins/info/sunset/SunsetPluginBase.h"

#include <QtGui/QWidget>

class SunsetPluginSettingsPane: public PluginSettingsPane
{
		Q_OBJECT

	public:
		SunsetPluginSettingsPane (SunsetPluginBase *plugin, QWidget *parent=NULL);
		~SunsetPluginSettingsPane();

	public slots:
		virtual void readSettings ();
		virtual bool writeSettings ();

	private slots:
		virtual void on_filenameInput_editingFinished ();
		virtual void on_findFileButton_clicked ();

	private:
		Ui::SunsetPluginSettingsPaneClass ui;

		SunsetPluginBase *plugin;
};

#endif
