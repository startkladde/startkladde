#ifndef _ShellPlugin_h
#define _ShellPlugin_h

#include <QObject>
#include <QString>

#include "src/accessor.h"

class QTextCodec;
class QLabel;
class QProcess;

class ShellPlugin: public QObject
{
		Q_OBJECT

	public:
		// Construction/Destruction
		void init ();
		ShellPlugin ();	// Initialize to default values
		ShellPlugin (const ShellPlugin &o);	// Copy constructor
		ShellPlugin (const QString desc);	// Initialize from config file QString
		ShellPlugin (const QString &_caption, const QString &_command, int interval);	// Initialize to given values
		virtual ~ShellPlugin ();
		ShellPlugin &operator= (const ShellPlugin &o);

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
		// TODO remove, use signals instead
		QLabel *caption_display;
		QLabel *value_display;

	private slots:
		void output_available ();
		void subprocess_died ();

	private:
		QTextCodec *codec;
};

// isRunning
#endif

