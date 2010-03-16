/*
 * ShellPluginInfo.h
 *
 *  Created on: 16.03.2010
 *      Author: Martin Herrmann
 */

#ifndef SHELLPLUGININFO_H_
#define SHELLPLUGININFO_H_

#include <QString>

class QSettings;

class ShellPluginInfo
{
	public:
		ShellPluginInfo ();
		ShellPluginInfo (QSettings &settings);
		ShellPluginInfo (const QString &caption, const QString &command, bool richText, int restartInterval, bool warnOnDeath);
		virtual ~ShellPluginInfo ();

		virtual QString toString () const;
		virtual operator QString () const;

		virtual void load (QSettings &settings);
		virtual void save (QSettings &settings) const;

		QString caption;
		QString command;
		bool richText;
		int restartInterval;	// Restart interval in seconds, 0 for no restart
		bool warnOnDeath;

	private:
		void initialize ();

};

#endif
