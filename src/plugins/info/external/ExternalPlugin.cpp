/*
 * ExternalPlugin.cpp
 *
 *  Created on: 22.07.2010
 *      Author: Martin Herrmann
 */

#include "ExternalPlugin.h"

//#include <QDebug>
//#include <QTime>
#include <QSettings>

#include "src/plugin/info/InfoPluginFactory.h"
#include "ExternalPluginSettingsPane.h"
//#include "src/text.h"
#include "src/util/qString.h"

REGISTER_INFO_PLUGIN (ExternalPlugin)
SK_PLUGIN_DEFINITION (ExternalPlugin, "{2fbb91be-bde5-4fba-a3c7-69d7caf827a5}", "Extern",
	utf8 ("Empfängt Daten von einem externen Programm"))

ExternalPlugin::ExternalPlugin (const QString &caption, bool enabled, const QString &command, bool richText):
	InfoPlugin (caption, enabled),
	command (command), richText (richText)
{
}

ExternalPlugin::~ExternalPlugin ()
{
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

void ExternalPlugin::start ()
{
	outputText ("Command is "+command, richText?Qt::RichText:Qt::PlainText);
	// FIXME
}

void ExternalPlugin::terminate ()
{
	// FIXME
}

QString ExternalPlugin::configText () const
{
	return utf8 ("„%1“, %2 text").arg (command, richText?"rich":"plain");
}


//#include <QLabel>
//#include <QTimer>
//#include <QTextCodec>
//#include <QToolTip>
//#include <QProcess>
//#include <QFile>
//#include <QFileInfo>
//#include <QThread>
//
//#include "src/text.h"
//#include "src/util/qString.h"
//#include "src/config/Settings.h"
//#include "src/plugin/ShellPluginInfo.h"
//
//// Construction/destruction
//void ShellPlugin::init ()
//{
//	restart_interval=-1;
//	rich_text=false;
//	caption_display=NULL;
//	value_display=NULL;
//	warn_on_death=false;
//	subprocess=NULL;
//
//	// The plugin output encoding is UTF8, but it is interpreted as latin1 and
//	// converted to UTF-8 by QProcess. It may not be obvious that this
//	// conversion (codec->toUnicode) will work, but it does.
//	codec=QTextCodec::codecForName ("UTF-8");
//}
//
//ShellPlugin::ShellPlugin ()
//{
//	init ();
//	caption="Time:";
//	command="date +%H:%M";
//	restart_interval=-1;
//}
//
//ShellPlugin::~ShellPlugin ()
//{
//	if (subprocess) subprocess->terminate ();
//	QThread::yieldCurrentThread ();
//}
//
//void ShellPlugin::start ()
//{
//	if (subprocess)
//	{
//		subprocess->terminate ();
//		QThread::yieldCurrentThread ();
//		delete subprocess;
//	}
//
//	if (command.isEmpty ())
//	{
//		if (value_display)
//		{
//			value_display->setTextFormat (Qt::PlainText);
//			value_display->setText ("Keine Plugin-Datei angegeben.");
//			value_display->setToolTip ("");
//		}
//		return;
//	}
//
//	// Separate the command in a file name and paramters at the first ' '
//	QString command_file, command_parameters;
//	int first_space=command.indexOf (' ');
//	if (first_space<0)
//	{
//		command_file=command;
//		command_parameters.clear ();
//	}
//	else
//	{
//		// foo bar
//		// 0123456
//		command_file=command.left (first_space);
//		command_parameters=command.mid (first_space+1);
//	}
//
//	// Find the plugin file from the plugin path list.
//	QString command_file_dir, command_file_basename;
//	if (ShellPlugin::findFile (command_file, &command_file_dir, &command_file_basename).isEmpty ())
//	{
//		// The plugin file was not found.
//		emit pluginNotFound ();
//
//		if (value_display)
//		{
//			value_display->setTextFormat (Qt::PlainText);
//			value_display->setText ("Plugin \""+command_file+"\" nicht gefunden.");
//			value_display->setToolTip ("");
//		}
//	}
//	else
//	{
//		// If the plugin path was explicitly given (either relative or
//		// absolute), we execute the process from the current directory. If
//		// not, it was found in the plugin_path and we execute it from there.
//		QString working_dir;
//		QString binary_file;
//		if (!command_file.contains ('/'))
//		{
//			// Found in path
//			working_dir=command_file_dir;
//			binary_file="./"+command_file_basename;
//		}
//		else
//		{
//			working_dir=".";
//			binary_file=command_file;
//		}
//
//		QString complete_command=binary_file+" "+command_parameters;
//
//		if (value_display)
//			value_display->setToolTip (complete_command+" ["+working_dir+"]");
//
//		subprocess=new QProcess (this);
//		subprocess->setWorkingDirectory (working_dir);
//
////		QObject::connect (subprocess, SIGNAL (destroyed (QObject *)), subprocess, SLOT (kill ()));
//		QObject::connect (subprocess, SIGNAL (readyReadStandardOutput ()), this, SLOT (output_available ()));
//		QObject::connect (subprocess, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (subprocess_died ()));
//
//		if (value_display)
//			value_display->setText ("");
//
//#ifdef WIN32
//		// Windows does not obey the shebang and cannot launch .rb files (even
//		// though they are associated with the interpreter and can be launched
//		// directly from the command line). So we have add ruby to the command
//		// explicitly.
//		// We do this only on Windows because this enables us to use non-ruby
//		// plugins on other platforms (although this is not portable).
//		complete_command="ruby "+complete_command;
//#endif
//		//std::cout << "launching: " << complete_command << " in " << working_dir << std::endl;
//		subprocess->start (complete_command, QIODevice::ReadOnly);
//
//		if (!subprocess->waitForStarted ())
//		{
//#ifdef WIN32
//			// On Windows, we call ruby explicitly, so most probably it was not
//			// found
//			QString message="Fehler: Ruby nicht installiert oder nicht im Suchpfad";
//#else
//			// On other platforms, the plugin ist called directly
//			QString message="Fehler beim Starten des Plugins";
//#endif
//			if (value_display) value_display->setText (message);
//		}
//
//		subprocess->closeWriteChannel ();
//
//	}
//}
//
//void ShellPlugin::output_available ()
//{
//	if (!subprocess) return;
//
//	QString line;
//	while (line=codec->toUnicode (subprocess->readLine ().trimmed ().constData ()), !line.isEmpty ())
//	{
//		emit lineRead (line);
//
//		if (value_display)
//		{
//			// Let the plugins wrap, or else the window might get wider than the screen
////			if (rich_text) line="<nobr>"+line+"</nobr>";
//			value_display->setText (line);
//		}
//	}
//}
//
//void ShellPlugin::subprocess_died ()
//{
//	if (warn_on_death) std::cout << "The process for '" << caption << "' died." << std::endl;
//	if (restart_interval>0) QTimer::singleShot (restart_interval*1000, this, SLOT (start ()));
//}
//
//void ShellPlugin::terminate ()
//{
//	// Set the labels to NULL to make sure they are not used any more
//	caption_display=NULL;
//	value_display=NULL;
//
//	if (subprocess) subprocess->terminate ();
//}
//
//void ShellPlugin::restart ()
//{
//	// Don't call #terminate here, as it will set the labels to NULL
//	if (subprocess) subprocess->terminate ();
//	start ();
//}
//
///**
// * Finds the absolute location of a plugin
// *
// * @param filename
// * @param dir set to the directory if the file is found
// * @param basename ?
// * @return the (existing) file, or an empty string if not found
// */
//QString ShellPlugin::findFile (const QString &filename, QString *dir, QString *basename)
//{
//	if (isBlank (filename)) return "";
//
//	if (filename.indexOf ('/')>=0)
//	{
//		// The name contains a '/' which means that it is an absolute or relative
//		if (QFile::exists (filename))
//		{
//			// Find the last slash: it separates the path from the basename
//			// Absolute: /foo/bar/baz
//			// Relative: bar/baz
//			//           0123456
//			unsigned int last_slash_pos=filename.lastIndexOf ('/');
//			// last_slash_pos cannot be npos becase there is a slash
//			if (dir) *dir=filename.left (last_slash_pos);
//			if (basename) *basename=filename.mid (last_slash_pos+4);
//			return filename;
//		}
//		else
//			return "";
//	}
//	else if (Settings::instance ().pluginPaths.isEmpty ())
//	{
//		// Pugin path is empty
//		if (QFile::exists (QString ("./")+filename))
//		{
//			if (dir) *dir=".";
//			if (basename) *basename=filename;
//			return "./"+filename;
//		}
//		else
//			return "";
//	}
//	else
//	{
//		// So we have to search the plugin path.
//		{
//			QStringListIterator it (Settings::instance ().pluginPaths);
//			while (it.hasNext ())
//			{
//				QString path_entry=it.next ();
//
//				if (QFile::exists (path_entry+"/"+filename))
//				{
//					if (dir) *dir=path_entry;
//					if (basename) *basename=filename;
//					return path_entry+"/"+filename;
//				}
//			}
//		}
//
//		{
//			// Still not found - try prepending the program path directory
//			QFileInfo programFile (Settings::instance ().programPath);
//			QString programDir=programFile.path ();
//
//			QStringListIterator it (Settings::instance ().pluginPaths);
//			while (it.hasNext ())
//			{
//				QString path_entry=it.next ();
//
//				if (!path_entry.startsWith ("/"))
//				{
//					path_entry=programDir+"/"+path_entry;
//					if (QFile::exists (path_entry+"/"+filename))
//					{
//						if (dir) *dir=path_entry;
//						if (basename) *basename=filename;
//						return path_entry+"/"+filename;
//					}
//				}
//			}
//		}
//	}
//
//	// Nothing found.
//	return "";
//}
