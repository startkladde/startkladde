#ifndef sk_plugin_h
#define sk_plugin_h

/*
 * sk_plugin
 * martin
 * 2004-12-18
 */

#include <QString>
#include <iostream>
#include <cstdio>
#include <fstream>

#include <QLabel>
#include <QObject>
#include <QToolTip>
#include <QProcess>

#include "src/accessor.h"

// This is a shell plugin.

class sk_plugin: public QObject
{
		Q_OBJECT

	public:
		// Construction/Destruction
		void init ();
		sk_plugin ();	// Initialize to default values
		sk_plugin (const sk_plugin &o);	// Copy constructor
		sk_plugin (const QString desc);	// Initialize from config file QString
		sk_plugin (const QString &_caption, const QString &_command, int interval);	// Initialize to given values
		virtual ~sk_plugin ();
		sk_plugin &operator= (const sk_plugin &o);

		// Display
		RO_ACCESSOR (QString, caption)
		RO_ACCESSOR (QString, command)
		RO_ACCESSOR (bool, rich_text)
		RW_ACCESSOR (int, restart_interval)
		RO_ACCESSOR (bool, warn_on_death)

		// UI
		RW_P_ACCESSOR (QLabel, caption_display)
		RW_P_ACCESSOR (QLabel, value_display)

	public slots:
		virtual void start ();
		virtual void terminate ();
		virtual void restart ();

	signals:
		void lineRead (QString output);
		void pluginNotFound ();

	private:
		// Plugin properties
		QString caption;
		QString command;
		bool rich_text;
		int restart_interval;	// Restart interval in seconds, -1 for no restart
		bool warn_on_death;

		// Subprocess
		QProcess *subprocess;

		// UI
		QLabel *caption_display;
		QLabel *value_display;

	private slots:
		void output_available ();
		void subprocess_died ();

};

// isRunning
#endif

