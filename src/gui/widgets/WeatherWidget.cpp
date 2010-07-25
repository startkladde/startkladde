#include "WeatherWidget.h"

#include <QEvent>
#include <QImage>
#include <QMovie>
#include <QPainter>
#include <QRegExp>
#include <QResizeEvent>

#include "src/config/Settings.h" // TOOD remove dependency, set size from MainWindow
#include "src/util/qString.h" // remove

WeatherWidget::WeatherWidget (QWidget *parent):
	SkLabel (parent),
	movie (NULL)
{
	if (Settings::instance ().coloredLabels)
	{
		setAutoFillBackground (true);
		setPaletteBackgroundColor (QColor (127, 127, 127));
	}

	setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
	setWordWrap (true);
	setScaledContents (true);
	setTextFormat (Qt::RichText);
}

WeatherWidget::~WeatherWidget ()
{
}

void WeatherWidget::setImage (const QImage &image)
{
	QPixmap pixmap=QPixmap::fromImage (image);
	setPixmap (pixmap);
}

void WeatherWidget::loadMovie (QSharedPointer<QTemporaryFile> file)
{
	// Clear the contents so we can delete the movie
	setText ("");

	// Delete the old movie (if any), so it does not try to access the file
	// any more.
	delete movie;

	// Store a shared pointer to the temporary file. The temporary file will
	// be deleted when all shared pointers have been deleted. If there is an
	// old file, this will probably happen right now.
	movieFile=file;

	movie=new QMovie (movieFile->fileName (), QByteArray (), this);

	// FIXME: the valid check and the setSpeed should be in the plugin;
	// we probably need an SkMovie which stores the reference to the temporary
	// file
	if (!movie->isValid ())
	{
		setText ("Fehler beim Laden des Videos");
	}
	else
	{
		movie->setSpeed (200);

		// Set the movie
		setMovie (movie);
		movie->start ();
	}
}

bool WeatherWidget::loadImage (const QString &fileName)
{
	setWordWrap (false);

	// Create and load the image
	QImage image;
	if (!image.load (fileName)) return false;

	//image.smoothScale (width(), height (), QImage::ScaleMin);
	QPixmap pixmap=QPixmap::fromImage (image);
	setPixmap (pixmap);

	int height=Settings::instance ().weatherPluginHeight;
	setFixedSize (height*pixmap.width()/pixmap.height(), height);

	return true;
}

bool WeatherWidget::loadMovie (const QString &fileName)
{
	// NB: wenn hier WordWrap=true ist (zum Beispiel, weil vorher ein Text
	// gesetzt war), dann funktioniert die Movie-Darstellung nicht mehr, wenn
	// einmal ein Text gesetzt war: das Fenster, das nur ein Layout mit diesem
	// Widget enthält, wird auf Höhe 0 gesetzt.
	// Nach diesem Fehler kann man bequem 2 Stunden lang suchen.

	setWordWrap (false);

	// FIXME leak
	QMovie *movie=new QMovie (fileName);
	setMovie (movie);
	movie->start ();

	return true;
}

void WeatherWidget::setText (const QString &text)
{
	setWordWrap (true);
	QLabel::setText (text);
	adjustSize ();
}

void WeatherWidget::inputLine (QString line)
{
	if (line.startsWith ("[MSG]", Qt::CaseInsensitive))
	{
		QRegExp rx ("\\[MSG\\]\\s*\\[(.*)\\]" );
		rx.indexIn (line);
		if (rx.numCaptures ()>0)
		{
			QString text=rx.cap (1);
			// (\\)*\n ==> newline
			// Regexp escaping: (\\\\)*\\n ==> newline
			// C escaping: (\\\\\\\\)*\\\\n ==> \n
			setText (text.replace (QRegExp ("(\\\\\\\\)*\\\\n"), "\n").replace ("\\\\", "\\"));
		}
	}
	else if (line.startsWith ("[IMG]", Qt::CaseInsensitive))
	{
		QRegExp rx ("\\[IMG\\]\\s*\\[(.*)\\]" );
		rx.indexIn (line);
		if (rx.numCaptures ()>0)
		{
			QString filename=rx.cap (1);
			if (!loadImage (filename))
			{
				setText ("Grafik kann nicht\ngeladen werden:\n"+filename);
			}
		}
	}
	else if (line.startsWith ("[MOV]", Qt::CaseInsensitive))
	{
		QRegExp rx ("\\[MOV\\]\\s*\\[(.*)\\]" );
		rx.indexIn (line);
		if (rx.numCaptures ()>0)
		{
			QString filename=rx.cap (1);
			if (!loadMovie (filename))
			{
				setText ("Film kann nicht\ngeladen werden:\n"+filename);
			}
			// Note that we cannot delete the file at this point because Qt
			// reads from the file when the animation starts over.
		}
	}
}

void WeatherWidget::mouseDoubleClickEvent (QMouseEvent *e)
{
	(void)e;
	emit (doubleClicked ());
}

void WeatherWidget::pluginNotFound ()
{
	setText ("Plugin nicht gefunden");
}

void WeatherWidget::resizeEvent (QResizeEvent *e)
{
	QLabel::resizeEvent (e);
	emit sizeChanged (e->size ());
}

//////////////////////

// TODO wenn Bild angezeigt, Bild mitresizen

//void WeatherWidget::paintEvent (QPaintEvent *e)
//{
//	::paintEvent (e)
//	// Is an image loaded?
//	if (pixmap.size ()!=QSize (0, 0))
//	{
//		QPainter painter (this);
//		painter.setClipRect (e->rect ());
////		painter.drawPixmap (0, 0, pixmap);
//		painter.drawPixmap (0, 0, scaledPixmap);
//	}
//}

