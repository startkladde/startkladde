#ifndef KML_H_
#define KML_H_

#include <QColor>
#include <QString>
#include <QList>
#include <QPen>

#include "src/numeric/GeoPosition.h"

namespace Kml
{
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

	class GroundOverlay
	{
		public:
			QString name;
			QString filename;
			Angle north, south, east, west;
			Angle rotation;
	};

	class Style
	{
		public:
			QColor labelColor;
			QColor lineColor;
			double lineWidth;

			Style ();
			QPen linePen () const;
	};

	class StyleMap
	{
		public:
			QMap<QString, QString> styles;
	};

	QColor parseColor (const QString color);
}


#endif
