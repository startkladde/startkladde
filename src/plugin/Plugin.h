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
 */
class Plugin: public QObject
{
		// FIXME Q_OBJECT with start, terminate and restart slot
	public:
		Plugin ();
		virtual ~Plugin ();

		// FIXME const
		// FIXME use a guid for Id?
		virtual QString getId          () const=0;
		virtual QString getName        () const=0;
		virtual QString getDescription () const=0;

		virtual void start ()=0;
		virtual void terminate ()=0;

		virtual QWidget *createSettingsPane ()=0;

		// FIXME rename read/write
		virtual void loadSettings (const QSettings &settings)=0;
		virtual void saveSettings (QSettings &settings)=0;
};

#endif
