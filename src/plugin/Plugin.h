#ifndef PLUGIN_H_
#define PLUGIN_H_

#include <qobject.h>

class QWidget;
class PluginSettingsPane;
class QSettings;

/**
 * A plugin
 *
 * A plugin is capable of loading and saving settings to a QSettings object and
 * can create a QWidget for configuration of the plugin.
 */
class Plugin: public QObject
{
		Q_OBJECT

	public:
		Plugin ();
		virtual ~Plugin ();

		// FIXME use a guid for Id?
		virtual QString getId          () const=0;
		virtual QString getName        () const=0;
		virtual QString getDescription () const=0;

	public slots:
		virtual void start ()=0;
		virtual void terminate ()=0;
		virtual void restart ();

		virtual PluginSettingsPane *createSettingsPane (QWidget *parent=NULL)=0;

		virtual void readSettings (const QSettings &settings)=0;
		virtual void writeSettings (QSettings &settings)=0;
};

#endif
