#ifndef LONGITUDE_H_
#define LONGITUDE_H_

#include "src/accessor.h"

#include <QString>

class Longitude
{
	public:
		Longitude ();
		Longitude (unsigned int degrees, unsigned int minutes, unsigned int seconds, bool positive);
		virtual ~Longitude ();

		value_accessor (unsigned int, Degrees, degrees);
		value_accessor (unsigned int, Minutes, minutes);
		value_accessor (unsigned int, Seconds, seconds);
		bool_accessor (Positive, positive);

		QString format (const QString &eastString="E", const QString &westString="W") const;

		QString toString () const;
		static Longitude fromString (const QString &string, bool *ok=NULL);

		Longitude normalized () const;

		double minusDegrees (const Longitude &other) const;

	private:
		unsigned int degrees;
		unsigned int minutes;
		unsigned int seconds;
		bool positive;
};

#endif
