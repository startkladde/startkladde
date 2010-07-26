#ifndef EXTERNALINFOPLUGIN_H_
#define EXTERNALINFOPLUGIN_H_

#include "src/plugin/info/InfoPlugin.h"

class QProcess;

/**
 * Example command: "date +%H:%M"
 */
class ExternalInfoPlugin: public InfoPlugin
{
		Q_OBJECT
		SK_PLUGIN

	public:
		ExternalInfoPlugin (const QString &caption=QString (), bool enabled=true, const QString &command="", bool richText=false);
		virtual ~ExternalInfoPlugin ();

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
