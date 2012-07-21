#include "src/flarm/KmlReader.h"

#include <iostream>

#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QFile>
#include <QIODevice>
#include <QStringList>

#include "src/util/qString.h"

KmlReader::KmlReader ()
{
}

KmlReader::~KmlReader ()
{
}

void KmlReader::readMarker (const QString &name, const QDomElement &lookAtElement)
{
	 double longitude=lookAtElement.firstChildElement ("longitude").text ().toDouble ();
	 double latitude =lookAtElement.firstChildElement ("latitude" ).text ().toDouble ();

	 KmlReader::Marker marker;
	 marker.position=GeoPosition::fromDegrees (latitude, longitude);
	 marker.color=Qt::red; // FIXME
	 marker.name=name;

	 markers.append (marker);
}

void KmlReader::readPath (const QString &name, const QDomElement &lineStringElement)
{
	 QList<GeoPosition> points;
	 QDomElement coordinatesElement=lineStringElement.firstChildElement ("coordinates");
	 foreach (const QString &pointCoordinates, coordinatesElement.text ().trimmed ().split (" "))
	 {
		 QStringList parts=pointCoordinates.split (",");
		 // FIXME only if exists
		 double longitude=parts[0].toDouble ();
		 double latitude =parts[1].toDouble ();
		 points.append (GeoPosition::fromDegrees (latitude, longitude));
	 }

	 KmlReader::Path path;
	 path.color=Qt::blue; // FIXME
	 path.lineWidth=1; // FIXME
	 path.name=name;
	 path.positions=points;

	 paths.append (path);
}

void KmlReader::readPolygon (const QString &name, const QDomElement &polygonElement)
{
	QList<GeoPosition> points;
	QDomElement coordinatesElemnt=polygonElement
			.firstChildElement ("outerBoundaryIs")
			.firstChildElement ("LinearRing")
			.firstChildElement ("coordinates");
	foreach (const QString &pointCoordinates, coordinatesElemnt.text ().trimmed ().split (" "))
	{
		QStringList parts=pointCoordinates.split (",");
		// FIXME only if exists
		double longitude=parts[0].toDouble ();
		double latitude =parts[1].toDouble ();
		points.append (GeoPosition::fromDegrees (latitude, longitude));
	}

	KmlReader::Polygon polygon;
	polygon.color=Qt::green; // FIXME
	polygon.lineWidth=2; // FIXME
	polygon.name=name;
	polygon.positions=points;

	polygons.append (polygon);
}

void KmlReader::readPlacemark (const QDomNode &node)
{
	 // FIXME only if it exists, or can we use it as a null element if not?
	 QString placemarkName=node.firstChildElement ("name").text ();
	 std::cout << "Got a placemark: " << placemarkName << std::endl;

	 QDomElement lookAtElement=node.firstChildElement ("LookAt");
	 if (!lookAtElement.isNull ())
		 readMarker (placemarkName, lookAtElement);

	 QDomElement lineStringElement=node.firstChildElement ("LineString");
	 if (!lineStringElement.isNull ())
		 readPath (placemarkName, lineStringElement);

	 QDomElement polygonElement=node.firstChildElement ("Polygon");
	 if (!polygonElement.isNull ())
		 readPolygon (placemarkName, polygonElement);
}

/**
 * This method is not reentrant because it calls a non-reentrant method of
 * QDomDocument.
 *
 * @param filename
 */
void KmlReader::read (const QString &filename)
{
	std::cout << "Reading KML file " << filename << std::endl;

	QDomDocument document ("kmlDocument");

	QFile file (filename);
	// FIXME error indication
	if (!file.open (QIODevice::ReadOnly))
		return;

	 if (!document.setContent (&file))
	     return;

	 file.close();

	 QDomNodeList placemarkNodes=document.elementsByTagName ("Placemark");

	 // FIXME clean up
	 // FIXME honor color, line thickness, other attributes?
	 for (int i=0, n=placemarkNodes.size (); i<n; ++i)
		 readPlacemark (placemarkNodes.at (i));
}
