#include "startkladde.h"

#include "sk_db.h"
#include "sk_win_main.h"
#include "options.h"
#include <unistd.h>
#include "sk_plugin.h"
#include "text.h"
#include <list>
#include "splash.h"
#include "logo.xpm"

// Testen des Wetterplugins
//#include "WeatherDialog.h"
//#include "sk_plugin.h"

void display_help ()/*{{{*/
	/*
	 * Displays a brief parameter information.
	 */
{
	cout << "usage: startkladde [options]" << endl;
	cout << "  options:" << endl;
	options::display_options ("    ");
}/*}}}*/

int main (int argc, char **argv)
	/*
	 * Starts the startkladde program.
	 * Parameters:
	 *   - argc, argv: the usual. Some of the options can be passed here. 
	 * Return value:
	 *   the return value of the QApplication, and thus of the main window.
	 */
{
	opts.parse_arguments (argc, argv);

	sk_db db;
	list<sk_plugin> plugins;

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
		if (!opts.style.empty ()) a.setStyle (std2q (opts.style));

		// Testen des Wetterplugins
		//sk_plugin *weather_ani_plugin=new sk_plugin ("Wettertitel", "plugins/weather/regenradar_wetteronline.de_ani", 600);	// Initialize to given values
		//WeatherDialog *weatherDialog;
		//weatherDialog=new WeatherDialog (weather_ani_plugin);
		//weatherDialog->setCaption ("Moobert");
		//weatherDialog->show ();
		//return a.exec ();

		sk_win_main w (NULL, &db, &plugins, "sk_win_main", Qt::WType_TopLevel);
		a.setMainWidget (&w);

		// Let the plugins initialize
		sched_yield ();

		db.set_connection_data (opts.server, opts.port, opts.username, opts.password);
		db.set_database (opts.database);
		db.display_queries=opts.display_queries;

		w.showMaximized ();
		w.start_db ();
		int ret=a.exec();

		return ret;
	}
}

