/*
 * ExternalWeatherPlugin.cpp
 *
 *  Created on: 22.07.2010
 *      Author: Martin Herrmann
 */

#include "ExternalWeatherPlugin.h"

//#include <QDebug>
#include <QFile>
#include <QRegExp>
#include <QImage>
#include <QMovie>

#include "src/plugin/factory/PluginFactory.h"
#include "src/io/SkProcess.h"
#include "src/text.h"
#include "src/util/qString.h"
//#include "src/util/io.h"
#include "src/config/Settings.h"
#include "src/graphics/SkMovie.h"

REGISTER_PLUGIN (WeatherPlugin, ExternalWeatherPlugin)
SK_PLUGIN_DEFINITION (ExternalWeatherPlugin, "{01db73ff-1473-4aeb-b297-13398927005c}", "Extern",
	utf8 ("Externes Wetter-Plugin"))

ExternalWeatherPlugin::ExternalWeatherPlugin (const QString &command):
	command (command),
	process (new SkProcess (this))
{
	connect (process, SIGNAL (lineReceived (const QString &)), this, SLOT (lineReceived (const QString &)));
	connect (process, SIGNAL (exited (int, QProcess::ExitStatus)), this, SLOT (processExited (int, QProcess::ExitStatus)));
}

ExternalWeatherPlugin::~ExternalWeatherPlugin ()
{
	terminate ();
}

void ExternalWeatherPlugin::refresh ()
{
	outputText (utf8 ("Prozess starten..."));

	if (isBlank (command)) OUTPUT_AND_RETURN ("Kein Kommando angegeben");

	QString commandProper;
	QString parameters;
	SkProcess::splitCommand (commandProper, parameters, command);

	QString resolved=resolveFilename (commandProper, Settings::instance ().pluginPaths);
	if (isBlank (resolved)) OUTPUT_AND_RETURN ("Kommando nicht gefunden");
	if (!QFile::exists (resolved)) OUTPUT_AND_RETURN ("Kommando existiert nicht");

	if (!process->startAndWait (resolved+" "+parameters)) OUTPUT_AND_RETURN (QString ("Fehler: %1").arg (process->getProcess ()->errorString ()));
	outputText ("Prozess gestartet");
}

void ExternalWeatherPlugin::abort ()
{
	process->stop ();
}


void ExternalWeatherPlugin::lineReceived (const QString &line)
{
	if (line.startsWith ("[MSG]", Qt::CaseInsensitive))
	{
		QRegExp rx ("\\[MSG\\]\\s*\\[(.*)\\]" );
		rx.indexIn (line);
		if (rx.numCaptures ()>0)
		{
			QString text=rx.cap (1);
			// An even number of backslashes, followed by a backslash and an n ==> newline
			// (\\)*\n ==> newline
			// Regexp escaping: (\\\\)*\\n ==> newline
			// C escaping: (\\\\\\\\)*\\\\n ==> \n
			outputText (text.replace (QRegExp ("(\\\\\\\\)*\\\\n"), "\n").replace ("\\\\", "\\"));
		}
	}
	else if (line.startsWith ("[IMG]", Qt::CaseInsensitive))
	{
		QRegExp rx ("\\[IMG\\]\\s*\\[(.*)\\]" );
		rx.indexIn (line);
		if (rx.numCaptures ()>0)
		{
			QString filename=rx.cap (1);
			QImage image (filename);

			if (image.isNull ())
				outputText ("Grafik kann nicht\ngeladen werden:\n"+filename);
			else
				outputImage (image);
		}
	}
	else if (line.startsWith ("[MOV]", Qt::CaseInsensitive))
	{
		QRegExp rx ("\\[MOV\\]\\s*\\[(.*)\\]" );
		rx.indexIn (line);
		if (rx.numCaptures ()>0)
		{
			QString filename=rx.cap (1);
			SkMovie movie (filename);
			if (movie.getMovie ()->isValid ())
				outputMovie (movie);
			else
				outputText ("Animation kann nicht\ngeladen werden:\n"+filename);
		}
	}
}

void ExternalWeatherPlugin::processExited (int exitCode, QProcess::ExitStatus exitStatus)
{
	(void)exitCode;
	(void)exitStatus;

	// That's cool
}
