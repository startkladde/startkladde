#ifndef WEATHERPLUGIN_H_
#define WEATHERPLUGIN_H_

#include <QUuid>

#include "src/plugin/Plugin.h"
#include "src/accessor.h"

class QImage;

/**
 * A plugin which displays a weather image (or an error message text)
 */
class WeatherPlugin: public Plugin
{
	Q_OBJECT

	public:
		// ***********
		// ** Types **
		// ***********

		class Descriptor
		{
			public:
				virtual WeatherPlugin *create () const=0;
				virtual QUuid   getId          () const=0;
				virtual QString getName        () const=0;
				virtual QString getDescription () const=0;

				static bool nameLessThan (const WeatherPlugin::Descriptor &d1, const WeatherPlugin::Descriptor &d2);
				static bool nameLessThanP (const WeatherPlugin::Descriptor *d1, const WeatherPlugin::Descriptor *d2);

			private:
				QString id, name, description;
		};

		template<class T> class DefaultDescriptor: public Descriptor
		{
			public:
				virtual WeatherPlugin *create () const { return new T (); }
				virtual QUuid   getId          () const { return T::_getId          (); }
				virtual QString getName        () const { return T::_getName        (); }
				virtual QString getDescription () const { return T::_getDescription (); }
		};


		// ******************
		// ** Construction **
		// ******************

		WeatherPlugin ();
		virtual ~WeatherPlugin ();


	signals:
		void textOutput (const QString &text, Qt::TextFormat format);
		void imageOutput (const QImage &image);

	protected:
		void outputText (const QString &text, Qt::TextFormat format=Qt::PlainText);
		void outputImage (const QImage &image);
};

#endif
