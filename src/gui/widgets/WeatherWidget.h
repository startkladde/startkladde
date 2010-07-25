#ifndef WEATHERWIDGET_H_
#define WEATHERWIDGET_H_

#include <QPixmap>
#include <QImageReader>
#include <QSharedPointer>
#include <QTemporaryFile> // Required for QSharedPointer deletion

#include "src/gui/widgets/SkLabel.h"

class QImage;
class QMovie;
class QTemporaryFile;

class WeatherWidget:public SkLabel
{
	Q_OBJECT

	public:
		WeatherWidget (QWidget *parent=NULL);
		virtual ~WeatherWidget ();
		virtual bool loadImage (const QString&);
		virtual bool loadMovie (const QString&);
		virtual void setText (const QString&);

	public slots:
		virtual void inputLine (QString line);
		virtual void setImage (const QImage &image);
		virtual void loadMovie (QSharedPointer<QTemporaryFile> file);

	signals:
		void doubleClicked ();
		void sizeChanged (const QSize&);

	protected:
		virtual void mouseDoubleClickEvent (QMouseEvent *e);
		virtual void resizeEvent (QResizeEvent *);

	protected slots:
		virtual void pluginNotFound ();

	private:
		QSharedPointer<QTemporaryFile> movieFile;
		QMovie *movie;
};

#endif

