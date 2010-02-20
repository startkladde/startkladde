#include "WeatherWidget.h"

#include <QEvent>
#include <QMovie>
#include <QPainter>
#include <QRegExp>
#include <QResizeEvent>

#include "src/config/Options.h" // TOOD remove dependency, set color from MainWindow

WeatherWidget::WeatherWidget (QWidget *parent, const char *name)
	:QLabel (parent, name)
{
	if (opts.colorful)
	{
		setAutoFillBackground (true);
		setPaletteBackgroundColor (QColor (127, 127, 127));
	}

	setAlignment (Qt::AlignHCenter | Qt::AlignVCenter | Qt::WordBreak);
	setScaledContents (true);
	setTextFormat (Qt::RichText);
}

bool WeatherWidget::loadImage (const QString &fileName)
{
	setWordWrap (false);

	// Create and load the image
	QImage image;
	if (!image.load (fileName)) return false;

	//image.smoothScale (width(), height (), QImage::ScaleMin);
	QPixmap pixmap (image);
	setPixmap (pixmap);

	setFixedSize (opts.weather_height*pixmap.width()/pixmap.height(), opts.weather_height);


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
	if (line.startsWith ("[MSG]", false))
	{
		QRegExp rx ("\\[MSG\\]\\s*\\[(.*)\\]" );
		rx.search (line);
		if (rx.numCaptures ()>0)
		{
			QString text=rx.cap (1);
			// (\\)*\n ==> newline
			// Regexp escaping: (\\\\)*\\n ==> newline
			// C escaping: (\\\\\\\\)*\\\\n ==> \n
			setText (text.replace (QRegExp ("(\\\\\\\\)*\\\\n"), "\n").replace ("\\\\", "\\"));
		}
	}
	else if (line.startsWith ("[IMG]", false))
	{
		QRegExp rx ("\\[IMG\\]\\s*\\[(.*)\\]" );
		rx.search (line);
		if (rx.numCaptures ()>0)
		{
			QString filename=rx.cap (1);
			if (!loadImage (filename))
			{
				setText ("Grafik kann nicht\ngeladen werden:\n"+filename);
			}
		}
	}
	else if (line.startsWith ("[MOV]", false))
	{
		QRegExp rx ("\\[MOV\\]\\s*\\[(.*)\\]" );
		rx.search (line);
		if (rx.numCaptures ()>0)
		{
			QString filename=rx.cap (1);
			if (!loadMovie (filename))
			{
				setText ("Film kann nicht\ngeladen werden:\n"+filename);
			}
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

