#ifndef NMEA_H_
#define NMEA_H_

#include <stdint.h>

#include <QDate>
#include <QTime>
#include <QDateTime>

#include "src/numeric/Angle.h"

class QString;

class Nmea
{
	public:
		virtual ~Nmea ();

	    static uint8_t calculateChecksum (const QString &sentence);
	    static bool verifyChecksum       (const QString &sentence);

	    static Angle parseLatitude  (const QString &value, const QString &sign);
	    static Angle parseLongitude (const QString &value, const QString &sign);

	    static QDate parseDate (const QString &value);
	    static QTime parseTime (const QString &value);
	    static QDateTime parseDateTime (const QString &dateValue, const QString &timeValue);

	private:
		Nmea ();

		static Angle parseAngle (const QString &value, const QString sign, int degreeDigits, const QString &positiveSign, const QString &negativeSign);
};

#endif
