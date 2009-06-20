#ifndef splash_h
#define splash_h

#include <qdialog.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qnamespace.h>
#include <iostream>
#include <qdesktopwidget.h>
#include <qevent.h>
#include "version.h"

using namespace std;
using namespace Qt;

/*
 * A splash screen capable of displaying a progress indicator
 */

class splash:public QDialog
{
	Q_OBJECT

	public:
		splash (QWidget *, const char * const *logo);
		void info ();

	private:
		QLabel *lbl_status;
		QLabel *lbl_picture;
		QLabel *lbl_version;
		bool display_status;
		bool can_close;
		QString status_text;

		QString i_string, n_string;

	
	protected:
		void keyPressEvent (QKeyEvent *);
	
	public slots:
		void show_splash ();
		void show_version ();
		void hide_splash ();
		void set_status (QString);
		void set_progress (int, int);
		void try_close ();
		void close ();

	signals:
		void clicked ();

	protected:
		 virtual void mousePressEvent (QMouseEvent * e);
};

#endif

