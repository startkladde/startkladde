#ifndef GEOPOSITION_H_
#define GEOPOSITION_H_

#include <QPointF>
#include <QDebug>

#include "src/numeric/Angle.h"

/**
 * A position on the earth
 *
 * Currently, neither range checking nor normalization of the latitude and
 * longitude are performed. Longitudes can be of any magnitude, and positions
 * with different longitudes can refer to the same point.
 *
 * A position can be invalid, as determined by the isValid method. Currently,
 * this only relates to whether values have been set, since no range checking
 * is performed.
 */
class GeoPosition
{
	public:
		// Mean earth radius in meters
		// http://en.wikipedia.org/wiki/Earth_radius
		static const double earthRadius = 6371.0*1000;

		GeoPosition ();
		GeoPosition (const Angle &latitude, const Angle &longitude);
		virtual ~GeoPosition ();

		static GeoPosition fromDegrees (const double latitude, const double longitude);
		static GeoPosition fromRadians (const double latitude, const double longitude);

		Angle getLatitude  () const { return latitude ; }
		Angle getLongitude () const { return longitude; }

		QPointF relativePositionTo (const GeoPosition &reference) const;

		bool isValid () const;

	private:
		Angle latitude, longitude;
};

QDebug operator<< (QDebug dbg, const GeoPosition &position);

#endif
