#include <unistd.h>

#include <QApplication>

#include "src/text.h"
#include "src/config/Options.h"
#include "src/db/Database.h"
#include "src/gui/windows/MainWindow.h"
#include "src/gui/windows/SplashScreen.h"
#include "src/plugins/ShellPlugin.h"

// Testen des Wetterplugins
//#include "WeatherDialog.h"
//#include "ShellPlugin.h"

void display_help ()
	/*
	 * Displays a brief parameter information.
	 */
{
	std::cout << "usage: startkladde [options]" << std::endl;
	std::cout << "  options:" << std::endl;
	Options::display_options ("    ");
}

int main (int argc, char **argv)
	/*
	 * Starts the startkladde program.
	 * Parameters:
	 *   - argc, argv: the usual. Some of the Options can be passed here.
	 * Return value:
	 *   the return value of the QApplication, and thus of the main window.
	 */
{
	opts.parse_arguments (argc, argv);

	// DbEvents are used as parameters for signals emitted by tasks running on
	// a background thread. These connections must be queued, so the parameter
	// types must be registered.
	qRegisterMetaType<DbEvent> ("DbEvent");
	qRegisterMetaType<DataStorage::State> ("DataStorage::State");

	Database db;
	QList<ShellPlugin *> plugins;

	if (opts.need_display ())
		opts.do_display ();
	else if (opts.display_help)
		display_help ();
	else
	{
		opts.read_config_files (&db, &plugins, argc, argv);
		//QApplication::setDesktopSettingsAware (FALSE); // I know better than the user
		QApplication a (argc, argv);

		// Put light.{la,so} to styles/
		//a.setStyle ("light, 3rd revision");
		if (!opts.style.isEmpty ()) a.setStyle (opts.style);

		db.display_queries=opts.display_queries;

		MainWindow w (NULL, &db, plugins);

		// Let the plugins initialize
		sched_yield ();



		w.showMaximized ();
//		w.show ();
		int ret=a.exec();

		foreach (ShellPlugin *plugin, plugins)
		{
//			std::cout << "Terminating plugin " << plugin->get_caption () << std::endl;
			plugin->terminate ();
			sched_yield ();
		}

		return ret;
	}
}

