#ifndef KMLREADER_H_
#define KMLREADER_H_

#include <cmath>

#include <QString>
#include <QColor>
#include <QMap>
#include <QPen>

#include "src/numeric/GeoPosition.h"

class QDomNode;
class QDomElement;



/**
 * A simple and special purpose KML file reader
 *
 * This uses the Qt Xml module.
 */
class KmlReader
{
	public:
		class Marker
		{
			public:
				GeoPosition position;
				QString name;
				QString styleUrl;
		};

		class Path
		{
			public:
				QString name;
				QList<GeoPosition> positions;
				QString styleUrl;
		};

		class Polygon
		{
			public:
				QString name;
				QList<GeoPosition> positions;
				QString styleUrl;
		};

		class Style
		{
			public:
				QColor labelColor;
				QColor lineColor;
				double lineWidth;

				Style (): lineWidth (1) {}

				QPen linePen ()
				{
					QPen pen (lineColor);
					pen.setWidth (round (lineWidth));
					return pen;
				}
		};

		class StyleMap
		{
			public:
				QMap<QString, QString> styles;
		};

		KmlReader ();
		virtual ~KmlReader ();

		void read (const QString &filename);

		KmlReader::Style findStyle (const QString &styleUrl);

		QList<KmlReader::Marker> markers;
		QList<KmlReader::Path> paths;
		QList<KmlReader::Polygon> polygons;

		QMap<QString, KmlReader::Style> styles;
		QMap<QString, KmlReader::StyleMap> styleMaps;

		QColor parseColor (const QString color);

	private:
		void readStyle (const QDomNode &styleNode);
		void readStyleMap (const QDomNode &styleMapNode);

		void readPlacemark (const QDomNode &placemarkNode);
		void readMarker  (const QString &name, const QString &styleUrl, const QDomElement &lookAtElement);
		void readPath    (const QString &name, const QString &styleUrl, const QDomElement &lineStringElement);
		void readPolygon (const QString &name, const QString &styleUrl, const QDomElement &polygonElement);
};

#endif
