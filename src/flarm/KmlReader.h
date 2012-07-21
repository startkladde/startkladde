#ifndef KMLREADER_H_
#define KMLREADER_H_

#include <QString>
#include <QColor>

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
				QColor color;
		};

		class Path
		{
			public:
				QString name;
				QList<GeoPosition> positions;
				QColor color;
				double lineWidth;
		};

		class Polygon
		{
			public:
				QString name;
				QList<GeoPosition> positions;
				QColor color;
				double lineWidth;
		};

		KmlReader ();
		virtual ~KmlReader ();

		void read (const QString &filename);

		QList<KmlReader::Marker> markers;
		QList<KmlReader::Path> paths;
		QList<KmlReader::Polygon> polygons;

	private:
		void readPlacemark (const QDomNode &node);
		void readMarker (const QString &name, const QDomElement &lookAtElement);
		void readPath (const QString &name, const QDomElement &lineStringElement);
		void readPolygon (const QString &name, const QDomElement &polygonElement);
};

#endif
