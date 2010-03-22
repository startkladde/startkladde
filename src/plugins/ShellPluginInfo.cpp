#include "ShellPluginInfo.h"

#include <QSettings>

ShellPluginInfo::ShellPluginInfo ()
{
	initialize ();
}

ShellPluginInfo::~ShellPluginInfo ()
{
}

ShellPluginInfo::ShellPluginInfo (QSettings &settings)
{
	initialize ();
	load (settings);
}

ShellPluginInfo::ShellPluginInfo (const QString &caption, const QString &command, bool richText, int restartInterval, bool warnOnDeath):
	caption (caption),
	command (command),
	richText (richText),
	restartInterval (restartInterval),
	warnOnDeath (warnOnDeath)
{
}

void ShellPluginInfo::initialize ()
{
	richText=false;
	restartInterval=60;
	warnOnDeath=false;
}

QString ShellPluginInfo::toString () const
{
	return QString ("%1: %2 (%3 s)").arg (caption, command).arg (restartInterval);
}

ShellPluginInfo::operator QString () const
{
	return toString ();
}

void ShellPluginInfo::load (QSettings &settings)
{
	caption        =settings.value ("caption"        , ""   ).toString ();
	command        =settings.value ("command"        , ""   ).toString ();
	richText       =settings.value ("richText"       , false).toBool ();
	restartInterval=settings.value ("restartInterval", 0    ).toInt ();
	warnOnDeath    =settings.value ("warnOnDeath"    , false).toBool ();
}

void ShellPluginInfo::save (QSettings &settings) const
{
	settings.setValue ("caption"        , caption        );
	settings.setValue ("command"        , command        );
	settings.setValue ("richText"       , richText       );
	settings.setValue ("restartInterval", restartInterval);
	settings.setValue ("warnOnDeath"    , warnOnDeath    );
}
