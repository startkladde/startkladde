#ifndef NMEA_H_
#define NMEA_H_

#include <stdint.h>

class QString;

class Nmea
{
	public:
		virtual ~Nmea ();

	    static uint8_t calculateChecksum (const QString &sentence);
	    static bool verifyChecksum       (const QString &sentence);

	private:
		Nmea ();
};

#endif
