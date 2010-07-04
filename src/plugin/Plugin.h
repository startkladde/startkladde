#ifndef PLUGIN_H_
#define PLUGIN_H_

#include <qobject.h>

class QWidget;
class QSettings;

/**
 * A plugin
 *
 * A plugin is capable of loading and saving settings to a QSettings object and
 * can create a QWidget for configuration of the plugin.
 *
 * In addition to the pure virtual methods, implementations of this class must
 * implement the following public static methods:
 *     static QString getName             ();
 *     static QString getShortDescription ();
 *     static QString getLongDescription  ();
 */
class Plugin: public QObject
{
	public:
		Plugin ();
		virtual ~Plugin ();

		virtual QWidget *createSettingsPane ()=0;

		virtual void loadSettings (const QSettings &settings)=0;
		virtual void saveSettings (const QSettings &settings)=0;
};

#endif
