/*
 * ExternalPlugin.cpp
 *
 *  Created on: 22.07.2010
 *      Author: Martin Herrmann
 */

/*
 * On process termination:
 *   - The process may finish spontaneously or because we terminated it.
 *   - When terminating the plugin, we want to terminate the process. Thus, we
 *     need to store a pointer to the QProcess as long as it is running
 *   - In the finished slot, the stored pointer may alredy have been
 *     overwritten (by restarting the plugin). In this case, we may not use it
 *     for deletion.
 */

/*
 * TODO:
 *   - parameters as a separate field (allow spaces!)
 *   - browse
 *   - restart behavior: no action, notify, restart (with interval), print message
 *   - working directory: current, program, plugin, other
 *   - allow specifying interpreter
 */

#include "ExternalPlugin.h"

//#include <QDebug>
#include <QSettings>
//#include <QTimer>
#include <QProcess>
#include <QFile>
#include <QString>

#include "src/plugin/info/InfoPluginFactory.h"
#include "ExternalPluginSettingsPane.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/util/io.h"
#include "src/config/Settings.h"

REGISTER_INFO_PLUGIN (ExternalPlugin)
SK_PLUGIN_DEFINITION (ExternalPlugin, "{2fbb91be-bde5-4fba-a3c7-69d7caf827a5}", "Extern",
	utf8 ("Empfängt Daten von einem externen Programm"))

ExternalPlugin::ExternalPlugin (const QString &caption, bool enabled, const QString &command, bool richText):
	InfoPlugin (caption, enabled),
	command (command), richText (richText),
	subprocess (NULL)
{
}

ExternalPlugin::~ExternalPlugin ()
{
	terminate ();
}

PluginSettingsPane *ExternalPlugin::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new ExternalPluginSettingsPane (this, parent);
}

void ExternalPlugin::infoPluginReadSettings (const QSettings &settings)
{
	command =settings.value ("command" , command ).toString ();
	richText=settings.value ("richText", richText).toBool ();
}

void ExternalPlugin::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue ("command" , command );
	settings.setValue ("richText", richText);
}

#define OUTPUT_AND_RETURN(text) do { outputText (utf8 (text)); return; } while (0)

void ExternalPlugin::start ()
{
	terminate ();

	if (isBlank (command)) OUTPUT_AND_RETURN ("Kein Kommando angegeben");

	QString commandProper;
	QString parameters;
	splitCommand (commandProper, parameters, command);

	QString resolved=resolveFilename (commandProper, Settings::instance ().pluginPaths);
	if (isBlank (resolved)) OUTPUT_AND_RETURN ("Kommando nicht gefunden");
	if (!QFile::exists (resolved)) OUTPUT_AND_RETURN ("Kommando existiert nicht");

	subprocess=new QProcess (this);
	// subprocess->setWorkingDirectory (...);

	connect (subprocess, SIGNAL (readyReadStandardOutput ()), this, SLOT (outputAvailable ()));
	//connect (subprocess, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (subprocess_died ()));
	connect (subprocess, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (processFinished ()));

	subprocess->start (resolved+" "+parameters, QIODevice::ReadOnly);
	if (!subprocess->waitForStarted ()) OUTPUT_AND_RETURN ("Fehler beim Starten des Prozesses");
	outputText ("Prozess gestartet");
	subprocess->closeWriteChannel ();

	// Note that on Windows, we may have to add the interpreter explicitly.
}

void ExternalPlugin::terminate ()
{
	if (subprocess)
	{
		if (subprocess->state ()==QProcess::NotRunning)
		{
			// The process is not running. We can delete it right away.
			delete subprocess;
		}
		else
		{
			// The process may still be running. Disconnect the regular
			// termination signal and connect the "delete on finished" signal.
			// The process may be deleted at any time (when it is finished), so
			// it is important to set the pointer to NULL.
			subprocess->disconnect (this);
			connect (subprocess, SIGNAL (finished ()), this, SLOT (deleteAfterFinished ()));
			subprocess->terminate ();
		}

		subprocess=NULL;
	}
}

QString ExternalPlugin::configText () const
{
	return utf8 ("„%1“, %2 text").arg (command, richText?"rich":"plain");
}

void ExternalPlugin::outputAvailable ()
{
	// Might happen if the process finished or was restarted in the meantime
	if (!subprocess) return;

	QString line;
	while (line=readLineUtf8 (*subprocess).trimmed (), !line.isEmpty ())
	{
		// FIXME: for the weather plugin, we must emit lineRead
		outputText (line, richText?Qt::RichText:Qt::PlainText);
	}
}

void ExternalPlugin::processFinished ()
{
	// The stored subprocess pointer may already have been overwritten, for
	// example, if the plugin has been restarted and this slot is called for
	// the "old" QProcess instance. Thus, we cannot use the stored pointer for
	// accessing the process and have to use the sender() instead.
	QProcess *p=dynamic_cast<QProcess *> (sender ());

	// In any case, whether the process was terminated or finished
	// spontaneously, we delete the QProcess instance. It may not be wise to
	// delete the sender of the signal that invoked a slot, so we defer
	// deletion to the event loop.
	p->deleteLater ();

	if (subprocess==p)
	{
		// We have a pointer to the process stored. This means that probably
		// the process finished spontaneously. Set the pointer to NULL so it
		// is not accessed later.
		// If the stored pointer points to something other than the finished
		// process, it is not touched.
		subprocess=NULL;
	}

	// Restarting (old code)
	// if (warn_on_death) std::cout << "The process for '" << caption << "' died." << std::endl;
	// if (restart_interval>0) QTimer::singleShot (restart_interval*1000, this, SLOT (start ()));
}

void ExternalPlugin::splitCommand (QString &commandProper, QString &parameters, const QString &commandWithParameters)
{
	int firstSpace=commandWithParameters.indexOf (' ');

	if (firstSpace<0)
	{
		// No space
		commandProper=commandWithParameters;
		parameters.clear ();
	}
	else
	{
		// foo bar
		// 0123456
		commandProper=commandWithParameters.left (firstSpace);
		parameters=commandWithParameters.mid (firstSpace+1);
	}
}
