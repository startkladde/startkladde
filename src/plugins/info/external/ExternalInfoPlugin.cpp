/*
 * ExternalInfoPlugin.cpp
 *
 *  Created on: 22.07.2010
 *      Author: Martin Herrmann
 */

/*
 * TODO:
 *   - parameters as a separate config field (allow spaces!)
 *   - restart behavior: no action, notify, restart (with interval), print message
 *   - working directory: current, program, plugin, other
 *   - allow specifying interpreter
 */

#include "ExternalInfoPlugin.h"

//#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QString>

#include "ExternalInfoPluginSettingsPane.h"
#include "src/plugin/factory/PluginFactory.h"
#include "src/io/SkProcess.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/config/Settings.h"

REGISTER_PLUGIN (InfoPlugin, ExternalInfoPlugin)
SK_PLUGIN_DEFINITION (ExternalInfoPlugin, "{2fbb91be-bde5-4fba-a3c7-69d7caf827a5}", "Extern",
	utf8 ("Empfängt Daten von einem externen Programm"))

ExternalInfoPlugin::ExternalInfoPlugin (const QString &caption, bool enabled, const QString &command, bool richText):
	InfoPlugin (caption, enabled),
	command (command), richText (richText),
	process (new SkProcess (this))
{
	connect (process, SIGNAL (lineReceived (const QString &)), this, SLOT (lineReceived (const QString &)));
	connect (process, SIGNAL (exited (int, QProcess::ExitStatus)), this, SLOT (processExited (int, QProcess::ExitStatus)));
}

ExternalInfoPlugin::~ExternalInfoPlugin ()
{
	terminate ();
}

PluginSettingsPane *ExternalInfoPlugin::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new ExternalInfoPluginSettingsPane (this, parent);
}

void ExternalInfoPlugin::infoPluginReadSettings (const QSettings &settings)
{
	command =settings.value ("command" , command ).toString ();
	richText=settings.value ("richText", richText).toBool ();
}

void ExternalInfoPlugin::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue ("command" , command );
	settings.setValue ("richText", richText);
}

void ExternalInfoPlugin::start ()
{
	terminate ();

	if (isBlank (command)) OUTPUT_AND_RETURN ("Kein Kommando angegeben");

	QString commandProper;
	QString parameters;
	SkProcess::splitCommand (commandProper, parameters, command);

	QString resolved=resolveFilename (commandProper, Settings::instance ().pluginPaths);
	if (isBlank (resolved)) OUTPUT_AND_RETURN ("Kommando nicht gefunden");
	if (!QFile::exists (resolved)) OUTPUT_AND_RETURN ("Kommando existiert nicht");

	if (!process->startAndWait (resolved+" "+parameters)) OUTPUT_AND_RETURN (QString ("Fehler: %1").arg (process->getProcess ()->errorString ()));
	outputText ("Prozess gestartet");

	// Note that on Windows, we may have to add the interpreter explicitly.
}

void ExternalInfoPlugin::terminate ()
{
	process->stop ();
}

QString ExternalInfoPlugin::configText () const
{
	return utf8 ("„%1“, %2 text").arg (command, richText?"rich":"plain");
}

void ExternalInfoPlugin::lineReceived (const QString &line)
{
	outputText (line, richText?Qt::RichText:Qt::PlainText);
}

void ExternalInfoPlugin::processExited (int exitCode, QProcess::ExitStatus exitStatus)
{
	(void)exitCode;
	(void)exitStatus;

	// Restarting (old code)
	// if (warn_on_death) std::cout << "The process for '" << caption << "' died." << std::endl;
	// if (restart_interval>0) QTimer::singleShot (restart_interval*1000, this, SLOT (start ()));
}
