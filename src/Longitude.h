#ifndef LONGITUDE_H_
#define LONGITUDE_H_

#include "src/accessor.h"

#include <QString>
#include <QTime>

/**
 * A longitude
 *
 * A longitude is represented as degrees, arc minutes, arc seconds and a sign.
 * The sign is positiv if the longitude is eastern and negative if the
 * longitude is western.
 *
 * The degree value can assume arbitrary values. The minute and second values
 * are in the 0..59 range.
 */
class Longitude
{
	public:
		Longitude ();
		Longitude (unsigned int degrees, unsigned int minutes, unsigned int seconds, bool positive);
		virtual ~Longitude ();

		value_reader (unsigned int, Degrees, degrees);
		value_reader (unsigned int, Minutes, minutes);
		value_reader (unsigned int, Seconds, seconds);
		bool_reader (Positive, positive);

		QString format (const QString &eastString="E", const QString &westString="W") const;

		QString toString () const;
		static Longitude fromString (const QString &string, bool *ok=NULL);

		Longitude normalized () const;

		double toDegrees () const;
		static Longitude fromDegrees (double deg);

		double minusDegrees (const Longitude &other) const;

	private:
		unsigned int degrees;
		unsigned int minutes;
		unsigned int seconds;
		bool positive;
};

#endif
