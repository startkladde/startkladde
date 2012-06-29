#include <src/numeric/GeoPosition.h>

#include <cmath>
#include <iostream>

#include <QString>

#include "src/util/qString.h"

/**
 * Constructs an invalid GeoPosition
 */
GeoPosition::GeoPosition ()
{
	// Since the two Angle properties are default-constructed, they will be
	// invalid and therefore this GeoPosition will also be invalid.
}

GeoPosition::~GeoPosition ()
{
}

/**
 * Creates a GeoPosition with the given latitude and longitude
 */
GeoPosition::GeoPosition (const Angle &latitude, const Angle &longitude):
	latitude (latitude), longitude (longitude)
{
}

/**
 * Creates a GeoPosition with the given latitude and longitude, specified in
 * degrees.
 *
 * As for precision:
 *   1 degree       = 1e-0 degree = 111 km (60 NM)
 *   0.1 degree     = 1e-1 degree = 11 km
 *   0.01 degree    = 1e-2 degree = 1 km
 *   0.001 degree   = 1e-3 degree = 111 m
 *   0.0001 degree  = 1e-4 degree = 11 m
 *   0.00001 degree = 1e-5 degree = 1 m
 */
GeoPosition GeoPosition::fromDegrees (const double latitude, const double longitude)
{
	return GeoPosition (Angle::fromDegrees (latitude), Angle::fromDegrees (longitude));
}

/**
 * Creates a GeoPosition with the given latitude and longitude, specified in
 * degrees.
 *
 * As for precision:
 *   1 rad          = 1e-0 rad = 6371 km
 *   0.1 rad        = 1e-1 rad =  637 km
 *   0.01 rad       = 1e-2 rad =   64 km
 *   0.001 rad      = 1e-3 rad =    7 km
 *   0.0001 rad     = 1e-4 rad = 6371 m
 *   0.00001 rad    = 1e-5 rad =  637 m
 *   0.000001 rad   = 1e-6 rad =   64 m
 *   0.0000001 rad  = 1e-7 rad =    7 m
 *   0.00000001 rad = 1e-8 rad =  0.7 m
 * @param latitude
 * @param longitude
 * @return
 */
GeoPosition GeoPosition::fromRadians (const double latitude, const double longitude)
{
	return GeoPosition (Angle::fromRadians (latitude), Angle::fromRadians (longitude));
}

/**
 * Calculates the relative position, in meters east and north of this position
 *
 * This only works for small differences in latitude. For large differences,
 * the relative position (as given here) is not well-defined.
 *
 * The east distance will be positive if this position is east of the reference.
 * Likewise, the north distance will be positive if this position is north of
 * the reference.
 *
 * Note that the east direction is defined as the direction where the longitude
 * does not wrap. For example, a point at 179°E would be considered 358° east of
 * a point at 179°W, even though it is more accurately described as 2° west.
 * Also, if the latitude parameters are out of range (±90°), the result will
 * be meaningless.
 *
 * @param reference the other point
 * @return a QPointF with the east distance as x and the north distance as y
 */
QPointF GeoPosition::relativePositionTo (const GeoPosition &reference) const
{
	Angle averageLatitude=(latitude+reference.latitude)/2;
	Angle latitudeDifference =latitude -reference.latitude;
	Angle longitudeDifference=longitude-reference.longitude;

	double east  = earthRadius * longitudeDifference.toRadians () * cos (averageLatitude.toRadians ());
	double north = earthRadius * latitudeDifference .toRadians ();

	return QPointF (east, north);
}

/**
 * Returns whether the position is valid
 *
 * Currently, this only checks whether values have been set; no range checking
 * is performed.
 */
bool GeoPosition::isValid () const
{
	// FIXME check the range? With a Latitude and Longitude class, this would
	// be included in this check.
	return latitude.isValid () && longitude.isValid ();
}

QDebug operator<< (QDebug dbg, const GeoPosition &position)
{
	dbg.nospace () << "(" << position.getLatitude ().toDegrees () << ", " << position.getLongitude ().toDegrees () << ")";
	return dbg.space ();
}
