#ifndef KMLREADER_H_
#define KMLREADER_H_

#include <QString>
#include <QColor>
#include <QMap>
#include <QPen>

#include "src/flarm/Kml.h"
#include "src/numeric/GeoPosition.h"

class QDomNode;
class QDomElement;

/**
 * A simple and special purpose KML file reader
 *
 * To use, create an instance and call read() with the name of a file to read.
 * You can then get the read data from the properties markers, paths and
 * polygons. You can also find a style with a given URL using findStyle, or you
 * can access styles and styleMaps directly.
 *
 * This uses the Qt Xml module.
 */
class KmlReader
{
	public:
		KmlReader ();
		virtual ~KmlReader ();

		void read (const QString &filename);

		Kml::Style findStyle (const QString &styleUrl);

		QList<Kml::Marker> markers;
		QList<Kml::Path> paths;
		QList<Kml::Polygon> polygons;

		QMap<QString, Kml::Style> styles;
		QMap<QString, Kml::StyleMap> styleMaps;

	private:
		void readStyle    (const QDomNode &styleNode);
		void readStyleMap (const QDomNode &styleMapNode);

		void readPlacemark (const QDomNode &placemarkNode);
		void readMarker  (const QString &name, const QString &styleUrl, const QDomElement &lookAtElement);
		void readPath    (const QString &name, const QString &styleUrl, const QDomElement &lineStringElement);
		void readPolygon (const QString &name, const QString &styleUrl, const QDomElement &polygonElement);
};

#endif
