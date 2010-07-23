#ifndef EXTERNALPLUGIN_H_
#define EXTERNALPLUGIN_H_

#include "src/plugin/info/InfoPlugin.h"

class QProcess;

/**
 * Example command: "date +%H:%M"
 */
class ExternalPlugin: public InfoPlugin
{
		Q_OBJECT
		SK_PLUGIN

	public:
		ExternalPlugin (const QString &caption=QString (), bool enabled=true, const QString &command="", bool richText=false);
		virtual ~ExternalPlugin ();

		virtual void start ();
		virtual void terminate ();

		virtual PluginSettingsPane *infoPluginCreateSettingsPane (QWidget *parent=NULL);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);

		value_accessor (QString, Command , command );
		value_accessor (bool   , RichText, richText);

		virtual QString configText () const;

	protected slots:
		void outputAvailable ();
		void processFinished ();

	protected:
		static void splitCommand (QString &commandProper, QString &parameters, const QString &commandWithParameters);

	private:
		QString command;
		bool richText;

		QProcess *subprocess;
};

#endif
