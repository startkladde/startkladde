#ifndef _WeatherWidget_h
#define _WeatherWidget_h

/*
 * WeatherWidget
 * martin
 * 2008-02-15
 */
#include <qlabel.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qimagereader.h>

using namespace std;

class WeatherWidget:public QLabel
{
	Q_OBJECT

	public:
		WeatherWidget (QWidget *parent=NULL, const char *name=NULL);
		virtual bool loadImage (const QString&);
		virtual bool loadMovie (const QString&);
		virtual void setText (const QString&);

	public slots:
		virtual void inputLine (QString line);

	signals:
		void doubleClicked ();
		void sizeChanged (const QSize&);

	protected:
		virtual void mouseDoubleClickEvent (QMouseEvent *e);
		virtual void resizeEvent (QResizeEvent *);

	protected slots:
		virtual void pluginNotFound ();
};

#endif

