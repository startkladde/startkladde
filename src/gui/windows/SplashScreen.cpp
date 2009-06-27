#include "SplashScreen.h"

#include <QKeyEvent>

#include "src/text.h"

// TODO multiple lines in message area
// TODO Klick auf labels schlie�en

SplashScreen::SplashScreen (QWidget *parent, const char * const *logo)
	:QDialog (parent, "splash screen", true, WStyle_Customize|Qt::WStyle_NoBorder|Qt::WStyle_StaysOnTop|Qt::WType_TopLevel)
	/*
	 * Creates a SplashScreen instance.
	 * Parameters:
	 *   - parent: passed to the base class constructor.
	 */
{
	QPixmap pixmap ((const char **)(logo));
	QPainter painter (this);

	display_status=true;
	can_close=false;

//	setPaletteBackgroundColor (QColor (0,0,0));


	lbl_picture=new QLabel (this);
    lbl_picture->setLineWidth (1);
    lbl_picture->setMidLineWidth (1);
	lbl_picture->setFrameStyle (QFrame::Panel | QFrame::Raised);
	lbl_picture->setPixmap (pixmap);
	// TODO use QLayout
	lbl_picture->move (0, 0);
	lbl_picture->adjustSize ();

	lbl_version=new QLabel (this);
    lbl_version->setLineWidth (1);
    lbl_version->setMidLineWidth (1);
	lbl_version->setFrameStyle (QFrame::Panel | QFrame::Sunken);
//	lbl_version->setFont (QFont ("Fixed[misc]", 9));
	lbl_version->setAutoFillBackground (true);
	{
	QPalette palette=lbl_version->palette ();
	palette.setColor (QPalette::Background, QColor (0, 0, 0));
	palette.setColor (QPalette::Foreground, QColor (255, 255, 255));
	lbl_version->setPalette (palette);
	}
	lbl_version->setText (version_info ());
	lbl_version->move (0, lbl_picture->height ());
	lbl_version->adjustSize ();
	lbl_version->resize (lbl_picture->width (), lbl_version->height ());

	lbl_status=new QLabel (this);
    lbl_status->setLineWidth (1);
    lbl_status->setMidLineWidth (1);
	lbl_status->setFrameStyle (QFrame::Panel | QFrame::Sunken);
//	lbl_status->setFont (QFont ("Fixed[misc]", 9));
	{
	QPalette palette=lbl_status->palette ();
	palette.setColor (QPalette::Background, QColor (0, 0, 0));
	palette.setColor (QPalette::Foreground, QColor (255, 255, 255));
	lbl_status->setPalette (palette);
	}
	lbl_status->setText ("Hauptflugbuch");
	lbl_status->move (0, lbl_picture->height ()+lbl_version->height ());
	lbl_status->adjustSize ();
	lbl_status->resize (lbl_picture->width (), lbl_status->height ());

	QObject::connect (this, SIGNAL (clicked ()), this, SLOT (try_close ()));
	resize (lbl_picture->width (), lbl_picture->height ()+lbl_version->height ()+lbl_status->height ());
}

void SplashScreen::try_close ()
	/*
	 * Close the window if can_close is set.
	 */
{
	if (can_close) close ();
}

void SplashScreen::close ()
	/*
	 * Close the window.
	 */
{
	done (0);
}

void SplashScreen::info ()
	/*
	 * Show as info dialog.
	 */
{
	display_status=false;
	can_close=true;

	// TODO move to resizeEvent (), use display_status.
	resize (lbl_picture->width (), lbl_picture->height ());
	setFocus ();
	show ();
	qApp->processEvents ();
}

void SplashScreen::show_splash ()
	/*
	 * Show as SplashScreen screen.
	 */
{
	// TODO progess indicator?
	resize (lbl_picture->width (), lbl_picture->height ()+lbl_version->height ()+lbl_status->height ());
	move ((qApp->desktop()->width ()-width ())/2, (qApp->desktop()->height ()-height ())/2);
	can_close=false;
	qApp->processEvents ();
	show ();
	qApp->processEvents ();
}

void SplashScreen::hide_splash ()
	/*
	 * Close the window.
	 */
{
	done (0);
}

void SplashScreen::show_version ()
	/*
	 * Display as version display.
	 */
{
	resize (lbl_picture->width (), lbl_picture->height ()+lbl_version->height ());
	can_close=true;
	show ();
	qApp->processEvents ();
	move ((qApp->desktop()->width ()-width ())/2, (qApp->desktop()->height ()-height ())/2);
//	setActiveWindow ();
	lbl_version->setFocus ();
//	exec ();
}



void SplashScreen::set_status (QString s)
	/*
	 * Sets the status text.
	 * Paramters:
	 *   - s: the text to set.
	 */
{
	std::cout << "Status: " << s << std::endl;
	status_text=s;
	lbl_status->setText (status_text);
	qApp->processEvents ();
}

void SplashScreen::set_progress (int i, int n)
	/*
	 * Sets the progress indicator.
	 * Parameters:
	 *   - i: the current value.
	 *   - n: the maximum value.
	 */
{
	i_string.setNum (i);
	n_string.setNum (n);
	lbl_status->setText (status_text+" ("+i_string+"/"+n_string+")");
	qApp->processEvents ();
}



void SplashScreen::keyPressEvent (QKeyEvent *e)
	/*
	 * A key was pressed. Handle it.
	 * Parameters:
	 *   - e: the key event passed by the QT library.
	 */
{
	switch (e->key ())
	{
		case Qt::Key_Escape:
			e->accept ();
			if (can_close) hide ();
			break;
		case Qt::Key_Return:
			e->accept ();
			if (can_close) hide ();
			break;
		default:
			e->ignore ();
			break;
	}

	if (can_close) hide ();

	// Hm. Is there something like QKeyEvent::accept () that also works for
	// the base class?
	if (!e->isAccepted ())
		QDialog::keyPressEvent (e);
}

void SplashScreen::mousePressEvent (QMouseEvent * e)
	/*
	 * A mouse event occured.
	 * Parameters:
	 *   - e: the mouse event passed by the QT library.
	 */
{
	(void)e;
	emit clicked ();
}

