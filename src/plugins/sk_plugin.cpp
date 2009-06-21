#include "sk_plugin.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

#include <QTimer>

#include "src/text.h"
#include "src/config/options.h"


// Construction/destruction/*{{{*/
void sk_plugin::init ()/*{{{*/
{
	restart_interval=-1;
	rich_text=false;
	caption_display=NULL;
	value_display=NULL;
	warn_on_death=false;
	subprocess=NULL;
}
/*}}}*/

sk_plugin::sk_plugin ()/*{{{*/
{
	init ();
	caption="Time:";
	command="date +%H:%M";
	restart_interval=-1;
}/*}}}*/

sk_plugin::sk_plugin (const sk_plugin &o)	// Copy constructor/*{{{*/
{
	(*this)=o;
}
/*}}}*/

sk_plugin::sk_plugin (const string desc)/*{{{*/
{
	init ();

	list<string> split;
	split_string (split, ",", desc);
	trim (split);

	list<string>::iterator end=split.end ();
	list<string>::iterator it=split.begin ();

	if (it!=end) caption=*it;
	if (++it!=end) command=*it;
	if (++it!=end) restart_interval=atoi ((*it).c_str ());
	while (++it!=end)
	{
		if ((*it)=="warn_on_death") warn_on_death=true;
		else if ((*it)=="rich_text") rich_text=true;
	}
	
	start ();
}
///*}}}*/

sk_plugin::sk_plugin (const string &_caption, const string &_command, int _interval)/*{{{*/
{
	init ();
	caption=_caption;
	command=_command;
	restart_interval=_interval;
}
/*}}}*/

sk_plugin &sk_plugin::operator= (const sk_plugin &o)/*{{{*/
{
	caption=o.caption;
	command=o.command;
	rich_text=o.rich_text;
	restart_interval=o.restart_interval;
	warn_on_death=o.warn_on_death;
	subprocess=NULL;
	caption_display=o.caption_display;
	value_display=o.value_display;
	return *this;
}
/*}}}*/

sk_plugin::~sk_plugin ()/*{{{*/
{
	// qt3
	//if (subprocess) subprocess->tryTerminate ();
	//qt4
	if (subprocess) subprocess->terminate ();
	sched_yield ();
}
///*}}}*/
/*}}}*/


void sk_plugin::start ()/*{{{*/
{
	if (subprocess) delete subprocess;
	if (command.empty ())
	{
		if (value_display)
		{
			value_display->setTextFormat (Qt::PlainText);
			value_display->setText ("Keine Plugin-Datei angegeben.");
			QToolTip::remove (value_display);
		}
		return;
	}

	// Separate the command in a file name and paramters at the first ' '
	string command_file, command_parameters;
	string::size_type first_space=command.find (' ');
	if (first_space==string::npos)
	{
		command_file=command;
		command_parameters.clear ();
	}
	else
	{
		// foo bar
		// 0123456
		command_file=command.substr (0, first_space);
		command_parameters=command.substr (first_space+1);
	}

	// Find the plugin file from the plugin path list.
	string command_file_dir, command_file_basename;
	if (opts.find_plugin_file (command_file, &command_file_dir, &command_file_basename).empty ())
	{
		// The plugin file was not found.
		emit pluginNotFound ();

		if (value_display) 
		{
			value_display->setTextFormat (Qt::PlainText);
			value_display->setText (std2q ("Plugin-Datei \""+command_file+"\" nicht gefunden."));
			QToolTip::remove (value_display);
		}
	}
	else
	{
		// If the plugin path was explicitly given (either relative or
		// absolute), we execute the process from the current directory. If
		// not, it was found in the plugin_path and we execute it from there.
		string working_dir;
		string binary_file;
		if (command_file.find ('/')==string::npos)
		{
			// Found in path
			working_dir=command_file_dir;
			binary_file="./"+command_file_basename;
		}
		else
		{
			working_dir=".";
			binary_file=command_file;
		}

		string complete_command=binary_file+" "+command_parameters;

		if (value_display)
		{
			QToolTip::add (value_display, std2q (complete_command+" ["+working_dir+"]"));
		}

		QStringList args;
		args.append ("/bin/sh");
		args.append ("-c");
		args.append (std2q (complete_command));

		// Qt3
		//subprocess=new QProcess (args, this, "subprocess");
//		subprocess->setWorkingDirectory (QDir (std2q (working_dir)));
		// Qt4
		subprocess=new QProcess (this);
		subprocess->setWorkingDirectory (std2q (working_dir));

		// Qt3
		//QObject::connect (subprocess, SIGNAL (destroyed (QObject *)), subprocess, SLOT (kill ()));
		//QObject::connect (subprocess, SIGNAL (readyReadStdout ()), this, SLOT (output_available ()));
		//QObject::connect (subprocess, SIGNAL (processExited ()), this, SLOT (subprocess_died ()));
		// Qt4
//		QObject::connect (subprocess, SIGNAL (destroyed (QObject *)), subprocess, SLOT (kill ()));
		QObject::connect (subprocess, SIGNAL (readyReadStandardOutput ()), this, SLOT (output_available ()));
		QObject::connect (subprocess, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (subprocess_died ()));

		

		if (value_display)
			value_display->setText ("");

		// Qt3
//		if (!subprocess->start ())
//		{
//			if (value_display)
//				value_display->setText ("Fehler beim Starten");
//		}
//		subprocess->closeStdin ();

		// Qt4
		subprocess->start ("/bin/sh -c \""+std2q (complete_command)+"\"", QIODevice::ReadOnly);
		subprocess->closeWriteChannel ();

	}
}
/*}}}*/

void sk_plugin::output_available ()/*{{{*/
{
	if (!subprocess) return;

	QString line;
	// Qt3
//	while (line=subprocess->readLineStdout (), !line.isNull ())
	// Qt4
	while (line=subprocess->readLine ().trimmed (), !line.isEmpty ())
	{
		emit lineRead (line);

		if (value_display)
		{
			// Let the plugins wrap, or else the window might get wider than the screen
//			if (rich_text) line="<nobr>"+line+"</nobr>";
			value_display->setText (line);
		}
	}
}
/*}}}*/

void sk_plugin::subprocess_died ()/*{{{*/
{
	if (warn_on_death) cout << "The process for '" << caption << "' died." << endl;
	if (restart_interval>=0) QTimer::singleShot (restart_interval*1000, this, SLOT (start ()));
}
/*}}}*/

void sk_plugin::terminate ()/*{{{*/
{
	// Qt3
	//if (subprocess) subprocess->tryTerminate ();
	// Qt4
	if (subprocess) subprocess->terminate ();
}
/*}}}*/

void sk_plugin::restart ()/*{{{*/
{
	terminate ();
	start ();
}
/*}}}*/

